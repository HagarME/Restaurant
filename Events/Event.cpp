 #include "Event.h"


Event::Event(int eTime, int ordID)
	: Time_Step(eTime), OrderID(ordID)
{
}


int Event::getEventTime() const
{
    return Time_Step;
}

int Event::getOrderID() const
{
    return OrderID;
}


Event::~Event()
{

}

