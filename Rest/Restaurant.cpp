#include "Restaurant.h"
#include "../Events/ArrivalEvent.h"
#include "../Events/CancellationEvent.h"
#include "../Events/PromotionEvent.h"
#include "../Rest/Cook.h"
#include <fstream>
#include <string>
#include <cmath>

Restaurant::Restaurant()
    : pGUI(nullptr),
      TotalWaitTime(0),
      TotalServTime(0),
      TotalTurnaround(0),
      CountFinished(0)
{
}

Restaurant::~Restaurant()
{
    if (pGUI) delete pGUI;
}

void Restaurant::UpdateServiceList(int CurrentTimeStep)
{
    Node<Order*>* curr = inService.getHead();

    while (curr)
    {
        Order* ord = curr->getItem();
        Cook* ck = ord->getCook();

        int serviceDuration =
            (ord->GetOrderSize() + ck->getCurrentSpeed() - 1) / ck->getCurrentSpeed();

        if (CurrentTimeStep - ord->GetServTime() >= serviceDuration)
        {
            // Finish order
            ord->setFinishTime(CurrentTimeStep);
            ord->setStatus(DONE);

            int waitTime = ord->GetServTime() - ord->GetArrTime();
            int turnaround = ord->GetFinishTime() - ord->GetArrTime();

            TotalWaitTime += waitTime;
            TotalServTime += serviceDuration;
            TotalTurnaround += turnaround;
            CountFinished++;

            // Free cook properly
            ck->finishCurrentOrder();

            Node<Order*>* toDelete = curr;
            curr = curr->getNext();

            inService.DeleteNodeByPointer(toDelete);
            finished.InsertEnd(ord);
        }
        else
        {
            curr = curr->getNext();
        }
    }
}


// The main simulation loop 
void Restaurant::RunSimulation()
{
    pGUI = new GUI();
    if (!pGUI) return;

    PROG_MODE mode = pGUI->getGUIMode();

    if (mode == MODE_INTR || mode == MODE_STEP || mode == MODE_SLNT || mode == MODE_DEMO)
    {
        string filename = "test.txt";

		// Interactive & Step-by-step and checking file existence and asking user for it
        if (mode == MODE_INTR || mode == MODE_STEP)
        {
            bool opened = false;
            while (!opened)
            {
                pGUI->PrintMessage("Enter input file name (e.g. test.txt): ");
                filename = pGUI->GetString();
                if (filename.empty()) filename = "test.txt";

                ifstream test(filename);
                if (test.is_open())
                {
                    test.close();
                    opened = true;
                    pGUI->PrintMessage("Loading: " + filename);
                }
                else
                {
                    pGUI->PrintMessage("File not found! Try again...");
                }
            }
        }

		// Demo mode and checking file existence and asking user for it
        if (mode == MODE_DEMO || mode == MODE_SLNT)
        {
            bool opened = false;
            while (!opened)
            {
                pGUI->PrintMessage("Enter input file name for DEMO (e.g. test.txt): ");
                filename = pGUI->GetString();
                if (filename.empty()) filename = "test.txt";

                ifstream test(filename);
                if (test.is_open())
                {
                    test.close();
                    opened = true;
                    pGUI->PrintMessage("Loading: " + filename);
                }
                else
                {
                    pGUI->PrintMessage("File not found! Try again...");
                }
            }
        }

        LoadInputFile(filename);

        int CurrentTimeStep = 1;

        while (true)
        {
            ExecuteEvents(CurrentTimeStep);

            //in this exact order
            CheckAutoPromotionOptimized(CurrentTimeStep);
            UpdateServiceList(CurrentTimeStep);
            AssignVIPOrders(CurrentTimeStep);      // Highest priority first
            AssignNormalOrders(CurrentTimeStep);    // Then Normal orders
            //then vegan orders...

            FillDrawingList();
            pGUI->UpdateInterface();
            pGUI->PrintMessage("Time Step: " + to_string(CurrentTimeStep));

            if (mode == MODE_INTR || mode == MODE_STEP)
                pGUI->waitForClick();
            else if (mode == MODE_DEMO)
            {
                clock_t delay = clock();
				while (clock() - delay < 400) {}  // for delay to visualize
            }

			// to finish simulation
            bool hasWaiting = !waitNormal.isEmpty() || !waitVegan.isEmpty() || !waitVIP.isEmpty();
            bool hasServing = !inService.isEmpty();
            bool hasFutureEvents = false;

            Node<Event*>* p = Events.getHead();
            while (p)
            {
                if (p->getItem()->getEventTime() >= CurrentTimeStep)
                {
                    hasFutureEvents = true;
                    break;
                }
                p = p->getNext();
            }

            if (!hasWaiting && !hasServing && !hasFutureEvents)
                break;

            CurrentTimeStep++;
        }

        pGUI->PrintMessage("Simulation Finished Successfully!");
        if (mode != MODE_SLNT)
            pGUI->waitForClick();
    }
}
void Restaurant::LoadInputFile(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        if (pGUI) pGUI->PrintMessage("ERROR: Cannot open file: " + filename);
        return;
    }
	// Read restaurant parameters N is number of cooks of each type , G is the number of vegan cooks , V is the number of VIP cooks
	// SN, SG, SV are the speed of normal , vegan , VIP cooks respectively
	// BO, BN, BG, BV are the break times for each type of cooks
	// M is the number of events
    int N, G, V, SN, SG, SV, BO, BN, BG, BV, M;
    file >> N >> G >> V;
    file >> SN >> SG >> SV;
    file >> BO >> BN >> BG >> BV;
    file >> M;  

    for (int i = 0; i < M; i++)
    {
        char eventType;
        file >> eventType;

        if (eventType == 'R')
        {
            char typ; int ts, id, size; double money;
            file >> typ >> ts >> id >> size >> money;
            ORD_TYPE type = (typ == 'N') ? TYPE_NRM : (typ == 'G') ? TYPE_VGAN : TYPE_VIP;
            Event* evt = new ArrivalEvent(ts, id, type, size, money);
            Events.InsertEnd(evt);
        }
        else if (eventType == 'X')
        {
            int ts, id;
            file >> ts >> id;
            Event* evt = new CancellationEvent(ts, id);
            Events.InsertEnd(evt);
        }
        else if (eventType == 'P')
        {
            int ts, id, extra;
            file >> ts >> id >> extra;
            Event* evt = new PromotionEvent(ts, id, extra);
            Events.InsertEnd(evt);
        }
    }
    pGUI->PrintMessage("Loaded " + to_string(Events.countNodes()) + " events. Starting simulation...");

    file.close();
}

void Restaurant::ExecuteEvents(int CurrentTimeStep)
{
    Node<Event*>* curr = Events.getHead();
    while (curr)
    {
        Event* e = curr->getItem();
        if (e->getEventTime() == CurrentTimeStep)
        {
            e->Execute(this);
            Node<Event*>* toDelete = curr;
            curr = curr->getNext();
            Events.DeleteNodeByPointer(toDelete);
        }
        else
        {
            curr = curr->getNext();
        }
    }
}

void Restaurant::MoveOneFromEachWaitToInService()
{
    if (!waitNormal.isEmpty())
    {
        Order* o = waitNormal.getHead()->getItem();
        o->setStatus(SRV);
        inService.InsertEnd(o);
        waitNormal.DeleteFirst();
    }
    if (!waitVegan.isEmpty())
    {
        Order* o = waitVegan.getHead()->getItem();
        o->setStatus(SRV);
        inService.InsertEnd(o);
        waitVegan.DeleteFirst();
    }
    if (!waitVIP.isEmpty())
    {
        Order* o = waitVIP.getHead()->getItem();
        o->setStatus(SRV);
        inService.InsertEnd(o);
        waitVIP.DeleteFirst();
    }
}
// Move one order from in-service to finished
void Restaurant::MoveOneFromInServiceToFinished()
{
    if (!inService.isEmpty())
    {
        Order* o = inService.getHead()->getItem();
        o->setStatus(DONE);
        finished.InsertEnd(o);
        inService.DeleteFirst();
    }
}
// Callbacks from Events
void Restaurant::AddToWaitingList(Order* pOrd)
{
    switch (pOrd->GetType())
    {
    case TYPE_NRM: waitNormal.InsertEnd(pOrd); break;
    case TYPE_VGAN: waitVegan.InsertEnd(pOrd); break;
    case TYPE_VIP: waitVIP.InsertEnd(pOrd); break;
    }
}
// Cancel order by ID
void Restaurant::CancelOrder(int orderID)
{
    Node<Order*>* curr = waitNormal.getHead();
    while (curr)
    {
        if (curr->getItem()->GetID() == orderID)
        {
            waitNormal.DeleteNodeByPointer(curr);
            return;
        }
        curr = curr->getNext();
    }
}
// GUI support 
void Restaurant::FillDrawingList()
{
    pGUI->ResetDrawingList();

    Node<Order*>* p;

    p = waitNormal.getHead(); while (p) { pGUI->AddToDrawingList(p->getItem()); p = p->getNext(); }
    p = waitVegan.getHead();  while (p) { pGUI->AddToDrawingList(p->getItem()); p = p->getNext(); }
    p = waitVIP.getHead();   while (p) { pGUI->AddToDrawingList(p->getItem()); p = p->getNext(); }

    p = inService.getHead();  while (p) { pGUI->AddToDrawingList(p->getItem()); p = p->getNext(); }
    p = finished.getHead();   while (p) { pGUI->AddToDrawingList(p->getItem()); p = p->getNext(); }

    // مفيش Cooks في Phase 1 خالص


}

//========================================
//========================================


void Restaurant::AssignVIPOrders(int currentTime)
{
    // Complexity: O(V + N + G) -> done once, not per order

    LinkedList<Cook*> availableVIP;
    LinkedList<Cook*> availableNormal;
    LinkedList<Cook*> availableVegan;

    // Scan VIP cooks
    Node<Cook*>* curr = vipCooks.getHead();

    while (curr)
    {
		//this whole loop is just because insertend acceps reference to pointer not pointer itself
        if (curr->getItem()->isAvailable())
        {
            Cook* VIPCookPtr = curr->getItem();
            availableVIP.InsertEnd(VIPCookPtr);
        }
        curr = curr->getNext();
    }

    // Scan Normal cooks
    curr = normalCooks.getHead();
    while (curr)
    {
        if (curr->getItem()->isAvailable())
        {
            Cook* NormalCookPtr = curr->getItem();
            availableNormal.InsertEnd(NormalCookPtr);
        }
        curr = curr->getNext();
    }

    // Scan Vegan cooks
    curr = veganCooks.getHead();
    while (curr)
    {
        if (curr->getItem()->isAvailable())
        {
            Cook* veganCookPtr = curr->getItem();
            availableVegan.InsertEnd(veganCookPtr);
        }
        curr = curr->getNext();
    }

    // Process VIP orders using pre-built lists
    // Complexity: O(VP × log VP) -> VP = VIP orders processed
    while (!waitVIP.isEmpty())
    {
        Order* vipOrder;
        int priority;

        if (!waitVIP.peek(vipOrder, priority))
            break;

        Cook* assignedCook = nullptr;

        // Try to get cook from available lists (O(1) each)
        if (!availableVIP.isEmpty())
        {
            assignedCook = availableVIP.GetHead();
            availableVIP.DeleteFirst();
        }
        else if (!availableNormal.isEmpty())
        {
            assignedCook = availableNormal.GetHead();
            availableNormal.DeleteFirst();
        }
        else if (!availableVegan.isEmpty())
        {
            assignedCook = availableVegan.GetHead();
            availableVegan.DeleteFirst();
        }

        // OPTIMIZATION 3: Only attempt preemption if no available cooks
        // and we have waiting VIP orders that need service
        if (!assignedCook)
        {
            // Preemption: O(N) but only when necessary
            Order* preemptedOrder = findNormalOrderToPreempt(currentTime);
            if (preemptedOrder)
            {
                assignedCook = findCookServingOrder(preemptedOrder);
                if (assignedCook)
                {
                    preemptOrder(assignedCook, preemptedOrder, currentTime);
                }
            }
        }

        // Assign if we found a cook
        if (assignedCook)
        {
            waitVIP.dequeue(vipOrder, priority);  // O(log W)
            assignedCook->assignOrder(vipOrder, currentTime);

            // Update statistics
            int waitTime = currentTime - vipOrder->GetArrTime();
            TotalWaitTime += waitTime;
        }
        else
        {
            // No cook available and no order to preempt
            // VIP order must wait until next timestep
            break;
        }
    }
}


// Helper: Find available cook of specific type
// Complexity: O(n) where n = number of cooks of that type
Cook* Restaurant::findAvailableCook(COOK_TYPE type)
{
    LinkedList<Cook*>* cookList = nullptr;

    switch (type)
    {
    case COOK_VIP:  cookList = &vipCooks; break;
    case COOK_NRM:  cookList = &normalCooks; break;
    case COOK_VGAN: cookList = &veganCooks; break;
    }

    if (!cookList) return nullptr;

    Node<Cook*>* current = cookList->getHead();
    while (current)
    {
        Cook* cook = current->getItem();
        if (cook->isAvailable())
            return cook;

        current = current->getNext();
    }

    return nullptr;  // No available cook of this type
}

// Find Normal order to preempt (choose least negative impact)
// Complexity: O(n) where n = number of busy Normal cooks
Order* Restaurant::findNormalOrderToPreempt(int currentTime)
{
    Order* bestToPreempt = nullptr;
    int leastServiceTime = INT_MAX;

    // Check all busy Normal cooks
    Node<Cook*>* current = normalCooks.getHead();
    while (current)
    {
        Cook* cook = current->getItem();

        // Only consider busy cooks serving Normal orders
        if (cook->isBusy())
        {
            Order* currentOrder = cook->getCurrentOrder();
            if (currentOrder && currentOrder->GetType() == TYPE_NRM)
            {
                // Calculate how much service time has been spent
                int startTime = currentOrder->GetServTime();
                int serviceTimeSoFar = currentTime - startTime;
                if (serviceTimeSoFar < 0) serviceTimeSoFar = 0;

                // Preempt the order with least service time (least waste)
                if (serviceTimeSoFar < leastServiceTime)
                {
                    leastServiceTime = serviceTimeSoFar;
                    bestToPreempt = currentOrder;
                }
            }
        }

        current = current->getNext();
    }

    return bestToPreempt;
}

// Find which cook is serving a specific order
// Complexity: O(total_cooks)
Cook* Restaurant::findCookServingOrder(Order* order)
{
    // Check all cook lists
    LinkedList<Cook*>* allLists[] = { &normalCooks, &veganCooks, &vipCooks };

    for (int i = 0; i < 3; i++)
    {
        Node<Cook*>* current = allLists[i]->getHead();
        while (current)
        {
            Cook* cook = current->getItem();
            if (cook->getCurrentOrder() == order)
                return cook;

            current = current->getNext();
        }
    }

    return nullptr;
}

// Complexity: O(1)
void Restaurant::preemptOrder(Cook* cook, Order* order, int currentTime)
{
    // Calculate remaining dishes
    int startTime = order->GetServTime();

    // Calculate how many ticks the cook worked on it
    int timeWorked = currentTime - startTime;
    if (timeWorked < 0) timeWorked = 0; // Safety check

    // Calculate dishes finished
    // Formula: Time * Speed
    int dishesCompleted = timeWorked * cook->getCurrentSpeed();

    // Safety Cap: You cannot finish more dishes than the total order size
    if (dishesCompleted > order->GetOrderSize())
    {
        dishesCompleted = order->GetOrderSize();
    }

    int remainingDishes = order->GetOrderSize() - dishesCompleted;

    // Update order size to remaining dishes
    order->setOrderSize(remainingDishes);

    // Remove order from cook
    cook->finishCurrentOrder();  // This frees the cook

    // Return order to Normal waiting list with ORIGINAL arrival time
    waitNormal.InsertEnd(order);
    order->setStatus(WAIT);

    if (pGUI)
    {
        pGUI->PrintMessage("Preempted Order " + to_string(order->GetID()) +
            ". Completed " + to_string(dishesCompleted) + " dishes.");
    }

    // When reassigned later, waiting time will be recalculated
}

void Restaurant::AssignNormalOrders(int currentTime)
{
    // Continue assigning while we have waiting Normal orders
    while (!waitNormal.isEmpty())
    {
        // Peek at the next Normal order
        // Complexity: O(1)
        Node<Order*>* frontNode = waitNormal.getHead();
        if (!frontNode) break;

        Order* normalOrder = frontNode->getItem();
        if (!normalOrder) break;

        Cook* assignedCook = nullptr;

        // Try Normal cooks first
        // Complexity: O(N)،،،، N = number of Normal cooks
        assignedCook = findAvailableCook(COOK_NRM);

        // If no Normal cook available, try VIP cooks
        // Complexity: O(V) where V = number of VIP cooks
        if (!assignedCook)
            assignedCook = findAvailableCook(COOK_VIP);

        //If we found an available cook, assign the order
        if (assignedCook)
        {
            // Remove from waiting list - O(1)
            waitNormal.DeleteFirst();

            // Assign order to cook - O(1)
            assignedCook->assignOrder(normalOrder, currentTime);

            // Calculate and store waiting time
            int waitTime = currentTime - normalOrder->GetArrTime();
            // You can store this in the order or accumulate statistics
            normalOrder->setServTime(currentTime);

            // Update statistics (in case)
            TotalWaitTime += waitTime;
        }
        else
        {
            // No cook available -> Normal order must wait
            // Exit loop (no more assignments possible this timestep
            break;
        }
    }
}



void Restaurant::CheckAutoPromotionOptimized(int currentTime)
{
    //  Keep track of oldest Normal order's arrival time
    // If oldest hasn't exceeded limit, none have

    if (waitNormal.isEmpty())
        return;

    // Peek at oldest order
    Node<Order*>* oldestNode = waitNormal.getHead();
    Order* oldestOrder = oldestNode->getItem();

    // Check oldest order first - O(1)
    int oldestWaitTime = currentTime - oldestOrder->GetArrTime();

    if (oldestWaitTime <= AutoP)
    {
        // If oldest hasn't exceeded limit, none have
        return;
    }

    // If we reach here, at least one order needs promotion
    // Scan and promote all that exceed limit - O(W_N)
    Node<Order*>* curr = waitNormal.getHead();

    while (curr)
    {
        Order* order = curr->getItem();
        int waitingTime = currentTime - order->GetArrTime();

        if (waitingTime > AutoP)
        {
            // This order needs promotion
            Node<Order*>* toPromote = curr;
            curr = curr->getNext();  // Move to next before deletion

            // Remove from Normal list
            Order* promotedOrder = toPromote->getItem();
            waitNormal.DeleteNodeByPointer(toPromote);

            // Add to VIP queue
            double priority = promotedOrder->calculateVIPPriority();
            waitVIP.enqueue(promotedOrder, (int)priority);

            autoPromotedCount++;

            if (pGUI)
            {
                pGUI->PrintMessage("Auto-promoted Order " +
                    to_string(promotedOrder->GetID()));
            }
        }
        else
        {
            //If the oldest person in the line hasn't waited long enough to be promoted yet, 
            //then the people behind them definitely haven't waited long enough either.
            break;
        }

    }

}