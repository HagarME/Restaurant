#include "PromotionEvent.h"
#include "../Final_phase1/Rest/Restaurant.h"

PromotionEvent::PromotionEvent(int eTime, int oID, int extraMoney)
    : Event(eTime, oID), ExtraMoney(extraMoney)
{
}

void PromotionEvent::Execute(Restaurant* pRest)
{
    // Phase 1: IGNORE PROMOTION EVENTS 
    
}