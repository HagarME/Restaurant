#include "CancellationEvent.h"
#include "../Final_phase1/Rest/Restaurant.h"

CancellationEvent::CancellationEvent(int eTime, int oID)
    : Event(eTime, oID)
{
}

void CancellationEvent::Execute(Restaurant* pRest)
{
    // Phase 1: Cancel only Normal orders from the waiting list
    pRest->CancelOrder(OrderID);
}