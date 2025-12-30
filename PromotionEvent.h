#ifndef __PROMOTION_EVENT_H_
#define __PROMOTION_EVENT_H_

#include "../Final_phase1/Events/Event.h"
#include "../Final_phase1/Rest/Restaurant.h"

class Restaurant; // Forward declaration

class PromotionEvent : public Event
{
protected:
    int ExtraMoney; // NOTE: Input uses INTEGER (e.g., "P 19 2 62")

public:
    PromotionEvent(int eTime, int oID, int extraMoney);
    virtual void Execute(Restaurant* pRest) override;
};

#endif