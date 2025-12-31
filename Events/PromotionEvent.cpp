#include "PromotionEvent.h"
#include "..\Rest\Restaurant.h"

PromotionEvent::PromotionEvent(int eTime, int ordID, int extraMoney)
    : Event(eTime, ordID), ExtraMoney(extraMoney)
{
}

void PromotionEvent::Execute(Restaurant* pRest)
{
    // Find and promote the Normal order to VIP
    pRest->PromoteOrder(OrderID, ExtraMoney);
}