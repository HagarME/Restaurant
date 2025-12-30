#include "Order.h"
// Constructor and Destructor 
Order::Order(int ID, ORD_TYPE r_Type)
    : ID(ID), type(r_Type), status(WAIT), Distance(0), totalMoney(0.0),
    ArrTime(0), ServTime(0), FinishTime(0), OrderSize(0)
{
}

Order::~Order() = default;

// --- Getters ---
int Order::GetID() const { 
return ID; 
}
ORD_TYPE Order::GetType() const {
return type;
}

ORD_STATUS Order::getStatus() const { 
return status;
}
int Order::GetDistance() const { 
return Distance; 
}
double Order::getTotalMoney() const {
return totalMoney;
}
int Order::GetArrTime() const {
return ArrTime;
}
int Order::GetServTime() const { 
return ServTime;
}
int Order::GetFinishTime() const { 
return FinishTime; 
}
int Order::GetOrderSize() const {
return OrderSize; 
}

// --- Setters ---
void Order::setStatus(ORD_STATUS s) {
status = s;
}
void Order::SetDistance(int d) { 
Distance = d;
}
void Order::setTotalMoney(double money) { 
totalMoney = money; 
}
void Order::setArrTime(int time) { 
ArrTime = time; 
}
void Order::setServTime(int time) { 
ServTime = time; 
}
void Order::setFinishTime(int time) {
FinishTime = time; 
}
void Order::setOrderSize(int size) {
OrderSize = size; 
}