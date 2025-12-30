#ifndef __CANCELLATION_EVENT_H_
#define __CANCELLATION_EVENT_H_

#include "Event.h"

class CancellationEvent : public Event
{
public:
    CancellationEvent(int eTime, int ordID);
    virtual void Execute(Restaurant* pRest) override;
};

#endif