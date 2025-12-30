#ifndef __PROMOTION_EVENT_H_
#define __PROMOTION_EVENT_H_

#include "Event.h"

class PromotionEvent : public Event
{
    int ExtraMoney;
public:
    PromotionEvent(int eTime, int ordID, int extraMoney);
    virtual void Execute(Restaurant* pRest) override;
};

#endif