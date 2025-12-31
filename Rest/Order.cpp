#include "Order.h"
#include "Cook.h"
// Constructor and Destructor 
Order::Order(int ID, ORD_TYPE r_Type)
    : ID(ID), type(r_Type), status(WAIT), Distance(0), totalMoney(0.0),
    ArrTime(0), ServTime(0), FinishTime(0), Deadline(0), isLate(false),
    OrderSize(0), assignedCook(nullptr)
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
int Order::getDeadline() const {
    return Deadline;
}
bool Order::getIsLate() const {
    return isLate;
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
void Order::setDeadline(int deadline) {
    Deadline = deadline;
}
void Order::setIsLate(bool late) {
    isLate = late;
}

//==================================
// VIP Priority calculation
double Order::calculateVIPPriority() const
{
    // Priority equation: Higher value = Higher priority
    // Here are the factors to understand the equation:
	// 1. Waiting time (current_time - arrival_time): increases urgency for a certain order
    // 2. Order price: higher paying customers get priority 
    // 3. Order size: smaller orders served faster (more customers happy)

    // Weights (we can change nums):  (determine how important a specific factor is compared to the others)
    const double W_TIME = 2.0;    // Waiting time weight
    const double W_PRICE = 0.5;   // Price weight
    const double W_SIZE = -0.3;   // Negative: smaller orders have higher priority

    // Normalize values to similar scales (adjusts the nums (like price) so they don't overpower the other factors)
    double timeComponent = W_TIME * ArrTime;           // Older orders = higher priority
    double priceComponent = W_PRICE * (totalMoney / 100.0);  // Normalize price
    double sizeComponent = W_SIZE * OrderSize;         // Smaller = higher priority

    return timeComponent + priceComponent + sizeComponent;
}

Cook* Order::getCook() const
{
    return assignedCook;
}

//==================================
// Deadline calculation: D = AT + f(SIZE, Price)
// Function: Base time based on size, with adjustment for price
// Higher price orders get slightly more time
int Order::calculateDeadline() const
{
    // Base deadline formula:
    // D = AT + (Size * 2) + (Price / 50)
    // Size factor: 2 timesteps per dish (reasonable cooking time)
    // Price factor: Higher paying customers get a bit more time
    
    double sizeComponent = OrderSize * 2.0;  // 2 timesteps per dish
    double priceComponent = totalMoney / 50.0;  // Normalize price impact
    
    int deadline = ArrTime + (int)(sizeComponent + priceComponent);
    
    return deadline;
}