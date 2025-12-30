#include "PromotionEvent.h"
#include "..\Rest\Restaurant.h"

PromotionEvent::PromotionEvent(int eTime, int ordID, int extraMoney)
    : Event(eTime, ordID), ExtraMoney(extraMoney)
{
}

void PromotionEvent::Execute(Restaurant* pRest)
{
    // Phase 1: promotions are ignored (no-op).
    // Implement promotion logic in later phases if required.
}