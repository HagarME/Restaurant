#ifndef __CANCELLATION_EVENT_H_
#define __CANCELLATION_EVENT_H_

#include "../Final_phase1/Rest/Order.h"
#include "../Final_phase1/Events/Event.h"

class Restaurant; // Forward declaration

class CancellationEvent : public Event
{
public:
    CancellationEvent(int eTime, int oID);
	// Execute the cancellation event by removing the order from the restaurant's waiting list
    virtual void Execute(Restaurant* pRest) override;
};

#endif