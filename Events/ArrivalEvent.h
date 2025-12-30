#ifndef __ARRIVAL_EVENT_H_
#define __ARRIVAL_EVENT_H_

#include "Event.h"
#include "..\Rest\Order.h" 


//class for the arrival event
class ArrivalEvent: public Event
{
	//info about the order ralted to arrival event
	int OrdDistance;	//order distance
	ORD_TYPE OrdType;		//order type: Normal, vegan, VIP	                
	double OrdMoney;	//Total order money
	int OrderSize;     // Number of dishes 
public:
	ArrivalEvent(int eTime, int oID, ORD_TYPE oType, int size, double money);
	
	virtual void Execute(Restaurant *pRest) override;	//override execute function

};

#endif