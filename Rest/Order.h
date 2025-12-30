#ifndef __ORDER_H_
#define __ORDER_H_

#include "..\Defs.h"

class Order
{
protected:
    int ID;                    // Each order has a unique ID (from 1 --> 999)
    ORD_TYPE type;             // Order type: Normal, Vegan, VIP
    ORD_STATUS status;         // WAIT, SRV, DONE
    int Distance;              // Distance (in meters) between order location and restaurant
    double totalMoney;         // Total order money

    int ArrTime, ServTime, FinishTime;  // Arrival, service start, and finish times

    // === Added as required by project specification ===
    int OrderSize;             // Number of dishes in the order 
public:
    // Constructor
    Order(int ID, ORD_TYPE r_Type);

    
    virtual ~Order();

    // --- Getters ---
    int GetID() const;
    ORD_TYPE GetType() const;
    ORD_STATUS getStatus() const;
    int GetDistance() const;
    double getTotalMoney() const;
    int GetArrTime() const;
    int GetServTime() const;
    int GetFinishTime() const;
    int GetOrderSize() const;  

    // --- Setters ---
    void setStatus(ORD_STATUS s);
    void SetDistance(int d);
    void setTotalMoney(double money);
    void setArrTime(int time);
    void setServTime(int time);
    void setFinishTime(int time);
    void setOrderSize(int size);  
};

#endif