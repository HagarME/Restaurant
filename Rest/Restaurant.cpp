#include "Restaurant.h"
#include "../Events/ArrivalEvent.h"
#include "../Events/CancellationEvent.h"
#include "../Events/PromotionEvent.h"
#include <fstream>
#include <string>

Restaurant::Restaurant() : pGUI(nullptr) {}

Restaurant::~Restaurant()
{
    if (pGUI) delete pGUI;
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
            MoveOneFromEachWaitToInService();

            if (CurrentTimeStep % 5 == 1)
                MoveOneFromInServiceToFinished();

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