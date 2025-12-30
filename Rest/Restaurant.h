#ifndef __RESTAURANT_H_
#define __RESTAURANT_H_

#include "..\Defs.h"
#include "..\GUI\GUI.h"
#include "..\LinkedList.h"
#include "..\Events\Event.h"
#include "Order.h"
#include "Cook.h"
#include <string>
#include "../priQueue.h"
#include "../LinkedQueue.h"
#include "../Rest/Cook.h"

class Restaurant
{
private:
    GUI* pGUI;

    // Events must be in a traversable list (not Queue)
    LinkedList<Event*> Events;

    // Waiting lists per order type
    LinkedList<Order*> waitNormal;
    LinkedQueue<Order*> waitVegan;  // FIFO for vegan
    priQueue<Order*> waitVIP;        // Priority Queue for VIP

    // In-Service and Finished lists (Phase 1 requirement)
    LinkedList<Order*> inService;
    LinkedList<Order*> finished;

    // Cook lists (loaded from input)
    LinkedList<Cook*> normalCooks;
    LinkedList<Cook*> veganCooks;
    LinkedList<Cook*> vipCooks;

    int AutoP; // Auto-promotion time limit


    void UpdateServiceList(int CurrentTimeStep);

    int TotalWaitTime;
    int TotalServTime;
    int TotalTurnaround;
    int CountFinished;


    // Helper functions
    void LoadInputFile(const std::string& filename);
    void ExecuteEvents(int currentTime);
    void MoveOneFromEachWaitToInService();
    void MoveOneFromInServiceToFinished();

public:
    Restaurant();
    ~Restaurant();

    void RunSimulation();

    // Callbacks from Events
    void AddToWaitingList(Order* pOrd);
    void CancelOrder(int orderID);

    // GUI support
    void FillDrawingList();
    void Just_A_Demo();	//just to show a demo and should be removed in phase1 1 & 2
    void AddtoDemoQueue(Order* po);	//adds an order to the demo queue

    void AssignVIPOrders(int currentTime);
    Cook* findAvailableCook(COOK_TYPE preferredType);
    Order* findNormalOrderToPreempt(int currentTime);
    Cook* findCookServingOrder(Order* order);
    void preemptOrder(Cook* cook, Order* order, int currentTime);
};


#endif
