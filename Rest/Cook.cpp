#include "Cook.h"
#include "Order.h"
#include <algorithm>
using namespace std;

Cook::Cook(int id, COOK_TYPE t, int baseSpd, int breakAft, int breakDur)
    : ID(id), type(t), baseSpeed(baseSpd), currentSpeed(baseSpd),
    status(AVAILABLE), currentOrder(nullptr),
    breakAfter(breakAft), breakDuration(breakDur),
    ordersServedSinceBreak(0), breakEndTime(-1), injuryEndTime(-1),
    totalOrdersServed(0), normalOrdersServed(0),
    veganOrdersServed(0), vipOrdersServed(0),
    totalBusyTime(0), totalIdleTime(0), totalBreakTime(0)
{
}

Cook::~Cook()
{
}

// Basic Getters (O(1))
int Cook::GetID() const { return ID; }
COOK_TYPE Cook::GetType() const { return type; }
int Cook::getSpeed() const { return currentSpeed; }
int Cook::getBaseSpeed() const { return baseSpeed; }
int Cook::getCurrentSpeed() const { return currentSpeed; }
COOK_STATUS Cook::getStatus() const { return status; }
Order* Cook::getCurrentOrder() const { return currentOrder; }

// Basic Getters (O(1))
bool Cook::isAvailable() const { return status == AVAILABLE; }
bool Cook::isBusy() const { return status == BUSY; }
bool Cook::isOnBreak() const { return status == ON_BREAK; }
bool Cook::isInjured() const { return status == INJURED; }

// Basic Setters (O(1))
void Cook::setID(int id) { ID = id; }
void Cook::setType(COOK_TYPE t) { type = t; }
void Cook::setSpeed(int s) { baseSpeed = s; currentSpeed = s; }

// Order Management (O(1))
void Cook::assignOrder(Order* pOrder, int currentTime)
{
    if (!pOrder || !isAvailable()) return;

    currentOrder = pOrder;
    status = BUSY;
    pOrder->setStatus(SRV);
    pOrder->setServTime(currentTime);
    pOrder->setCook(this);
}

Order* Cook::finishCurrentOrder()
{
    if (!currentOrder) return nullptr;

    Order* completedOrder = currentOrder;
    currentOrder = nullptr;
    status = AVAILABLE;

    // Update statistics based on order type
    totalOrdersServed++;
    ordersServedSinceBreak++;

    switch (completedOrder->GetType())
    {
    case TYPE_NRM:  normalOrdersServed++; break;
    case TYPE_VGAN: veganOrdersServed++;  break;
    case TYPE_VIP:  vipOrdersServed++;    break;
    }

    // Apply fatigue after completing order
    applyFatigue();

    return completedOrder;
}

// Break Management (O(1))
void Cook::startBreak(int currentTime)
{
    status = ON_BREAK;
    breakEndTime = currentTime + breakDuration;
    ordersServedSinceBreak = 0;

    // Restore speed during break
    restoreSpeed();
}

void Cook::endBreak()
{
    status = AVAILABLE;
    breakEndTime = -1;
}

bool Cook::needsBreak() const
{
    return (ordersServedSinceBreak >= breakAfter) && !isOnBreak();
}

// Injury Management (O(1))
void Cook::setInjured(int currentTime, int recoveryDuration)
{
    status = INJURED;
    injuryEndTime = currentTime + recoveryDuration;
}

void Cook::recover()
{
    status = AVAILABLE;
    injuryEndTime = -1;
}

// Fatigue System (O(1))
void Cook::applyFatigue()
{
    // Fatigue rule: reduce speed by 5% after each order (minimum 1)
    currentSpeed = max(1, (int)(currentSpeed * 0.95));
}

void Cook::restoreSpeed()
{
    // Full recovery during break
    currentSpeed = baseSpeed;
}

// Timestep Update (O(1))
void Cook::updateStatus(int currentTime)
{
    // break should end ?
    if (isOnBreak() && currentTime >= breakEndTime)
    {
        endBreak();
    }

    // injury recovery complete?
    if (isInjured() && currentTime >= injuryEndTime)
    {
        recover();
    }

    // Update statistics
    if (isBusy())
        totalBusyTime++;
    else if (isAvailable())
        totalIdleTime++;
    else if (isOnBreak() || isInjured())
        totalBreakTime++;
}

// Getters for statistics (O(1))
int Cook::getTotalOrdersServed() const { return totalOrdersServed; }
int Cook::getNormalOrdersServed() const { return normalOrdersServed; }
int Cook::getVeganOrdersServed() const { return veganOrdersServed; }
int Cook::getVIPOrdersServed() const { return vipOrdersServed; }
int Cook::getTotalBusyTime() const { return totalBusyTime; }
int Cook::getTotalIdleTime() const { return totalIdleTime; }
int Cook::getTotalBreakTime() const { return totalBreakTime; }

double Cook::getUtilization() const
{
    int total = totalBusyTime + totalIdleTime + totalBreakTime;
    if (total == 0) return 0.0;
    return (double)totalBusyTime / total * 100.0;
}