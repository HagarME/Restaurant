#include "Restaurant.h"
#include "../Events/ArrivalEvent.h"
#include "../Events/CancellationEvent.h"
#include "../Events/PromotionEvent.h"
#include "../Rest/Cook.h"
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>

Restaurant::Restaurant()
    : pGUI(nullptr),
      TotalWaitTime(0),
      TotalServTime(0),
      TotalTurnaround(0),
      CountFinished(0),
      lateOrderCount(0),
      autoPromotedCount(0),
      AutoP(0)
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
            
            // Calculate deadline and check if late
            int deadline = ord->calculateDeadline();
            ord->setDeadline(deadline);
            
            if (CurrentTimeStep > deadline)
            {
                ord->setIsLate(true);
                lateOrderCount++;
            }

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

            AssignVeganOrders(CurrentTimeStep);     // Then vegan orders...

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
            
            // Update all cook statuses each timestep
            Node<Cook*>* cookNode = normalCooks.getHead();
            while (cookNode)
            {
                cookNode->getItem()->updateStatus(CurrentTimeStep);
                cookNode = cookNode->getNext();
            }
            cookNode = veganCooks.getHead();
            while (cookNode)
            {
                cookNode->getItem()->updateStatus(CurrentTimeStep);
                cookNode = cookNode->getNext();
            }
            cookNode = vipCooks.getHead();
            while (cookNode)
            {
                cookNode->getItem()->updateStatus(CurrentTimeStep);
                cookNode = cookNode->getNext();
            }

            CurrentTimeStep++;
        }

        // Write output file
        WriteOutputFile("output.txt");
        
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
    file >> AutoP;  // Auto-promotion limit
    
    // Create Normal cooks
    for (int i = 1; i <= N; i++)
    {
        Cook* newCook = new Cook(i, COOK_NRM, SN, BO, BN);
        normalCooks.InsertEnd(newCook);
    }
    
    // Create Vegan cooks
    for (int i = 1; i <= G; i++)
    {
        Cook* newCook = new Cook(i, COOK_VGAN, SG, BO, BG);
        veganCooks.InsertEnd(newCook);
    }
    
    // Create VIP cooks
    for (int i = 1; i <= V; i++)
    {
        Cook* newCook = new Cook(i, COOK_VIP, SV, BO, BV);
        vipCooks.InsertEnd(newCook);
    }
    
    // Read number of events
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

//for bonus 1:
/*void Restaurant::LoadInputFile(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        if (pGUI) pGUI->PrintMessage("ERROR: Cannot open file: " + filename);
        return;
    }

    // Read number of cooks
    int N, G, V, BO;
    file >> N >> G >> V;
    file >> BO;  // Break Orders (same for all)

    // Read N Normal cooks
    for (int i = 0; i < N; i++)
    {
        string cookID;
        int speed, breakDuration;
        file >> cookID >> speed >> breakDuration;

        int id = i + 1;  // Or extract from cookID
        Cook* newCook = new Cook(id, COOK_NRM, speed, BO, breakDuration);
        normalCooks.InsertEnd(newCook);
    }

    // Read G Vegan cooks
    for (int i = 0; i < G; i++)
    {
        string cookID;
        int speed, breakDuration;
        file >> cookID >> speed >> breakDuration;

        int id = N + i + 1;
        Cook* newCook = new Cook(id, COOK_VGAN, speed, BO, breakDuration);
        veganCooks.InsertEnd(newCook);
    }

    // Read V VIP cooks
    for (int i = 0; i < V; i++)
    {
        string cookID;
        int speed, breakDuration;
        file >> cookID >> speed >> breakDuration;

        int id = N + G + i + 1;
        Cook* newCook = new Cook(id, COOK_VIP, speed, BO, breakDuration);
        vipCooks.InsertEnd(newCook);
    }

    // Sort each cook list by speed (descending - highest speed first)
    // Complexity: O(N log N) + O(G log G) + O(V log V) = O(C log C) where C = total cooks
    sortCooksBySpeed(normalCooks);
    sortCooksBySpeed(veganCooks);
    sortCooksBySpeed(vipCooks);

    // Read auto-promotion limit
    file >> AutoP;

    // Read events (same as before)
    int M;
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

    file.close();
}
*/

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
        Order* o = waitVegan.peek();
        o->setStatus(SRV);
        inService.InsertEnd(o);
        waitVegan.dequeue();
    }
    if (!waitVIP.isEmpty())
    {
        Order* o; int p;
        waitVIP.peek(o, p);
        o->setStatus(SRV);
        inService.InsertEnd(o);
        waitVIP.dequeue(o, p);  // Pass arguments for dequeue
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
    case TYPE_VGAN: waitVegan.enqueue(pOrd); break;
    case TYPE_VIP: 
        double priority = pOrd->calculateVIPPriority();
        waitVIP.enqueue(pOrd, (int)priority); 
        break;
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

// Promote Normal order to VIP by ID
// Complexity: O(W_N) where W_N = number of waiting Normal orders
void Restaurant::PromoteOrder(int orderID, int extraMoney)
{
    Node<Order*>* curr = waitNormal.getHead();
    while (curr)
    {
        Order* order = curr->getItem();
        if (order->GetID() == orderID)
        {
            // Remove from Normal waiting list
            waitNormal.DeleteNodeByPointer(curr);
            
            // Add extra money to order
            order->setTotalMoney(order->getTotalMoney() + extraMoney);
            
            // Convert to VIP type
            order->setType(TYPE_VIP);
            
            // Calculate VIP priority and add to VIP queue
            double priority = order->calculateVIPPriority();
            waitVIP.enqueue(order, (int)priority);
            
            if (pGUI)
            {
                pGUI->PrintMessage("Promoted Order " + to_string(orderID) + " to VIP");
            }
            
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
    // For priQueue, we cannot access nodes directly. Use getItem(i, ref)
    for (int i = 0; i < waitVIP.getSize(); i++) {
        Order* ord;
        if (waitVIP.getItem(i, ord))
            pGUI->AddToDrawingList(ord);
    }

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

        // Only attempt preemption if no available cooks
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
    if (timeWorked < 0) timeWorked = 0;

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



void Restaurant::AssignVeganOrders(int currentTime)
{
    while (!waitVegan.isEmpty())
    {
        Order* veganOrder = waitVegan.peek();
        Cook* assignedCook = findAvailableCook(COOK_VGAN);

        if (assignedCook)
        {
            waitVegan.dequeue();
            assignedCook->assignOrder(veganOrder, currentTime);
            int waitTime = currentTime - veganOrder->GetArrTime();
            TotalWaitTime += waitTime;
        }
        else
        {
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

//for bonus 1:
// Merge Sort for LinkedList
// Complexity: O(n log n) where n = number of cooks in the list
void Restaurant::sortCooksBySpeed(LinkedList<Cook*>& cookList)
{
    if (cookList.isEmpty() || cookList.getSize() <= 1)
        return;

    // Convert to array for easier sorting
    int count = cookList.getSize();
    Cook** arr = new Cook * [count];

    // Extract to array - O(n)
    Node<Cook*>* curr = cookList.getHead();
    int idx = 0;
    while (curr)
    {
        arr[idx++] = curr->getItem();
        curr = curr->getNext();
    }

    // Sort array by speed (descending) - O(n log n)
    mergeSortCooks(arr, 0, count - 1);

    // Clear original list - O(n)
    while (!cookList.isEmpty())
        cookList.DeleteFirst();

    // Rebuild list with sorted order - O(n)
    for (int i = 0; i < count; i++)
        cookList.InsertEnd(arr[i]);

    delete[] arr;
}

// Merge sort for cook array
void Restaurant::mergeSortCooks(Cook** arr, int left, int right)
{
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortCooks(arr, left, mid);
    mergeSortCooks(arr, mid + 1, right);
    mergeCooks(arr, left, mid, right);
}

//Merge two sorted subarrays
void Restaurant::mergeCooks(Cook** arr, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Cook** L = new Cook * [n1];
    Cook** R = new Cook * [n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int i = 0; i < n2; i++)
        R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;

    // Merge in descending order (higher speed first)
    while (i < n1 && j < n2)
    {
        if (L[i]->getCurrentSpeed() >= R[j]->getCurrentSpeed())
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    delete[] L;
    delete[] R;
}

// Write output file with all simulation results and statistics
// Must be called at end of simulation
// Complexity: O(N log N) where N = finished orders (for sorting)
void Restaurant::WriteOutputFile(const std::string& filename)
{
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        if (pGUI) pGUI->PrintMessage("ERROR: Cannot write to output file");
        return;
    }

    // Convert finished linked list to array for sorting
    int numOrders = finished.getSize();
    Order** orderArray = new Order*[numOrders];
    
    Node<Order*>* curr = finished.getHead();
    int index = 0;
    while (curr && index < numOrders)
    {
        orderArray[index++] = curr->getItem();
        curr = curr->getNext();
    }

    // Sort orders by FT, then by ST (bubble sort for simplicity)
    for (int i = 0; i < numOrders - 1; i++)
    {
        for (int j = 0; j < numOrders - i - 1; j++)
        {
            Order* o1 = orderArray[j];
            Order* o2 = orderArray[j + 1];
            
            int ft1 = o1->GetFinishTime();
            int ft2 = o2->GetFinishTime();
            int st1 = o1->GetServTime();
            int st2 = o2->GetServTime();
            
            // Sort by FT first, then by ST
            bool shouldSwap = false;
            if (ft1 > ft2)
                shouldSwap = true;
            else if (ft1 == ft2 && st1 > st2)
                shouldSwap = true;
                
            if (shouldSwap)
            {
                Order* temp = orderArray[j];
                orderArray[j] = orderArray[j + 1];
                orderArray[j + 1] = temp;
            }
        }
    }

    // Write header
    outFile << "FT\tID\tAT\tWT\tST\n";

    // Write sorted orders
    for (int i = 0; i < numOrders; i++)
    {
        Order* ord = orderArray[i];
        int ft = ord->GetFinishTime();
        int id = ord->GetID();
        int at = ord->GetArrTime();
        int wt = ord->GetServTime() - ord->GetArrTime();  // WT = ServTime - ArrTime
        int st = ft - ord->GetServTime();                  // ST = FinishTime - ServTime

        outFile << ft << "\t" << id << "\t" << at << "\t" 
                << wt << "\t" << st << "\n";
    }

    delete[] orderArray;

    // Count orders by type
    int normalCount = 0, veganCount = 0, vipCount = 0;
    curr = finished.getHead();
    while (curr)
    {
        ORD_TYPE type = curr->getItem()->GetType();
        if (type == TYPE_NRM) normalCount++;
        else if (type == TYPE_VGAN) veganCount++;
        else if (type == TYPE_VIP) vipCount++;
        curr = curr->getNext();
    }

    // Calculate averages
    double avgWait = (CountFinished > 0) ? (double)TotalWaitTime / CountFinished : 0.0;
    double avgServ = (CountFinished > 0) ? (double)TotalServTime / CountFinished : 0.0;

    // Count total cooks
    int totalCooks = normalCooks.getSize() + veganCooks.getSize() + vipCooks.getSize();

    // Write statistics
    outFile << "\nOrders: " << numOrders 
            << " [Norm:" << normalCount 
            << ", Veg:" << veganCount 
            << ", VIP:" << vipCount << "]\n";
    
    outFile << "Cooks: " << totalCooks
            << " [Norm:" << normalCooks.getSize()
            << ", Veg:" << veganCooks.getSize()
            << ", VIP:" << vipCooks.getSize() << "]\n";
    
    outFile << "Avg Wait = " << fixed << setprecision(2) << avgWait
            << ", Avg Serv = " << avgServ << "\n";
    
    outFile << "Auto-promoted: " << autoPromotedCount << "\n";
    outFile << "Late Orders: " << lateOrderCount << "\n";

    // Per-cook statistics
    // Normal cooks
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook N" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    // Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook G" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    // VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook V" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    outFile.close();
    
    if (pGUI)
        pGUI->PrintMessage("Output file written successfully: " + filename);
}

// ========================================
// Dynamic Behavior Methods
// ========================================

// Trigger breaks for cooks who have served BO consecutive orders
// Implements overtime/break skipping when system is overloaded
// Complexity: O(C) where C = total number of cooks
void Restaurant::TriggerCookBreaks(int currentTime)
{
    bool overloaded = isSystemOverloaded();
    
    // Check all Normal cooks
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                // OVERTIME: Cook skips break due to overload
                // Penalty: Apply extra fatigue (reduce speed more)
                cook->applyFatigue();  // Double fatigue penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                // Normal break
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check all Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                cook->applyFatigue();  // Overtime penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check all VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                cook->applyFatigue();  // Overtime penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
}

// Check if system is overloaded
// Condition: More than 5 VIP orders waiting
// Complexity: O(1)
bool Restaurant::isSystemOverloaded() const
{
    // System is overloaded if there are many waiting VIP orders
    // Threshold: 5 or more VIP orders waiting
    return waitVIP.getSize() >= 5;
}

// Trigger random health emergencies (injuries) for cooks
// Probability: 0.1% per cook per timestep (about 1 injury per 1000 timesteps per cook)
// Recovery duration: 10 timesteps
// Complexity: O(C) where C = total number of cooks
void Restaurant::TriggerRandomInjuries(int currentTime)
{
    // Random injury probability: 0.1% = 0.001
    // Using simple pseudo-random based on current time and cook ID
    
    // Check all cooks for potential injury
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        // Only available or busy cooks can get injured (not already injured/on break)
        if (cook->isAvailable() || cook->isBusy())
        {
            // Pseudo-random: if (time * 7 + id * 13) % 1000 == 0, trigger injury
            // This gives approximately 0.1% chance
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)  // 0.1% chance
            {
                // Injury triggered!
                int recoveryDuration = 10;  // 10 timesteps to recover
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->isAvailable() || cook->isBusy())
        {
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)
            {
                int recoveryDuration = 10;
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->isAvailable() || cook->isBusy())
        {
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)
            {
                int recoveryDuration = 10;
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
}

void Restaurant::AddVIPOrder(Order* order, int priority)
{
    waitVIP.enqueue(order, priority);
}
