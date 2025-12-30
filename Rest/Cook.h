#ifndef __COOK_H_
#define __COOK_H_

#include "..\Defs.h"

class Order;

enum COOK_STATUS
{
    AVAILABLE,    // Ready to take orders
    BUSY,         // Currently preparing an order
    ON_BREAK,     // Taking scheduled break
    INJURED       // Health emergency (unavailable)
};

class Cook
{
private:
    int ID;
    COOK_TYPE type;           // VIP, Normal, or Vegan

    // Speed tracking
    int baseSpeed;            // Base dishes per timestep (from input)
    int currentSpeed;         // Current speed (affected by fatigue)

    // Status tracking
    COOK_STATUS status;       // Current status
	Order* currentOrder;      // Pointer to order being prepared (if free, it'll be nullptr)

    // Break management
	int breakAfter;           // Orders before break (Break Orders (BO) from input)
    int breakDuration;        // Break duration in timesteps (Break Normal (BN) /BVip/ Break Vegan (BV) from input)
    int ordersServedSinceBreak;  // Counter for break
    int breakEndTime;         // When current break ends ( will output -1 if not on break)

    // Injury management
    int injuryEndTime;        // When recovery ends (will output -1 if not injured)

    // Statistics
    int totalOrdersServed;    // Total orders handled
    int normalOrdersServed;   // Normal orders count
    int veganOrdersServed;    // Vegan orders count
    int vipOrdersServed;      // VIP orders count
    int totalBusyTime;        // Time spent cooking
    int totalIdleTime;        // Time spent available but not assigned
    int totalBreakTime;       // Time spent on breaks/injury

public:
    // Constructor
    Cook(int id, COOK_TYPE t, int baseSpd, int breakAfter, int breakDur);
    virtual ~Cook();

    // Basic getters
    int GetID() const;
    COOK_TYPE GetType() const;
    int getSpeed() const;
    int getBaseSpeed() const;
    int getCurrentSpeed() const;
    COOK_STATUS getStatus() const;
    Order* getCurrentOrder() const;

    // Status checking (O(1) complexity)
    bool isAvailable() const;    // Can take new order now
    bool isBusy() const;         // Currently cooking
    bool isOnBreak() const;      // On scheduled break
    bool isInjured() const;      // Injured and recovering

    // Basic setters
    void setID(int id);
    void setType(COOK_TYPE t);
    void setSpeed(int s);

    // Order assignment (O(1) complexity)
    void assignOrder(Order* pOrder, int currentTime);
    Order* finishCurrentOrder();  // Returns completed order

    // Break management (O(1) complexity)
    void startBreak(int currentTime);
    void endBreak();
    bool needsBreak() const;

    // Injury management (O(1) complexity)
    void setInjured(int currentTime, int recoveryDuration);
    void recover();

    // Fatigue system (O(1) complexity)
    void applyFatigue();         // Called after each order
    void restoreSpeed();         // Called during breaks

    // Timestep update (O(1) complexity)
    void updateStatus(int currentTime);

	//To get just some statistics
    int getTotalOrdersServed() const;
    int getNormalOrdersServed() const;
    int getVeganOrdersServed() const;
    int getVIPOrdersServed() const;
    int getTotalBusyTime() const;
    int getTotalIdleTime() const;
    int getTotalBreakTime() const;
    double getUtilization() const;  // Busy / (Busy + Idle + Break)
};

#endif