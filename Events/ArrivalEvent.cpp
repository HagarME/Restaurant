#include "ArrivalEvent.h"
#include "..\Rest\Restaurant.h"


ArrivalEvent::ArrivalEvent(int eTime, int oID, ORD_TYPE oType, int size, double money)
	: Event(eTime, oID), OrdType(oType), OrdMoney(money), OrderSize(size), OrdDistance(0)
{
	// Note: OrdDistance is not provided in input → set to 0 (or compute later if needed)
}

void ArrivalEvent::Execute(Restaurant* pRest)
{
    // Create Order ONCE (as required by project: "allocate once, move don’t copy")
    Order* pOrder = new Order(OrderID, OrdType);
    pOrder->setArrTime(Time_Step);
    pOrder->setOrderSize(OrderSize);
    pOrder->setTotalMoney(OrdMoney);
    // Distance can be set later if needed; for now, leave as 0

    // In Phase 1: add to appropriate waiting list (no cook assignment)
    pRest->AddToWaitingList(pOrder);
}
