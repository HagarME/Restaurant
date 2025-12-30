#ifndef __RESTAURANT_H_
#define __RESTAURANT_H_

#include "..\Defs.h"
#include "..\GUI\GUI.h"
#include "..\LinkedList.h"
#include "..\Events\Event.h"
#include "Order.h"
#include "Cook.h"
#include <string>

class Restaurant
{
private:
    GUI* pGUI;

    // Events must be in a traversable list (not Queue)
    LinkedList<Event*> Events;

    // Waiting lists per order type (Phase 1 requirement)
    LinkedList<Order*> waitNormal;
    LinkedList<Order*> waitVegan;
    LinkedList<Order*> waitVIP;

    // In-Service and Finished lists (Phase 1 requirement)
    LinkedList<Order*> inService;
    LinkedList<Order*> finished;

    // Cook lists (loaded from input)
    LinkedList<Cook*> normalCooks;
    LinkedList<Cook*> veganCooks;
    LinkedList<Cook*> vipCooks;

    int AutoP; // Auto-promotion time limit

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
};

#endif