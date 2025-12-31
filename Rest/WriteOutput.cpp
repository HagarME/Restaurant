#include "Restaurant.h"
#include <iomanip>
#include <fstream>
// Must be called at end of simulation
// Complexity: O(N log N) where N = finished orders (for sorting)
void Restaurant::WriteOutputFile(const std::string& filename)
{
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        if (pGUI) pGUI->PrintMessage("ERROR: Cannot write to output file");
        return;
    }

    // Convert finished linked list to array for sorting
    int numOrders = finished.getSize();
    Order** orderArray = new Order*[numOrders];
    
    Node<Order*>* curr = finished.getHead();
    int index = 0;
    while (curr && index < numOrders)
    {
        orderArray[index++] = curr->getItem();
        curr = curr->getNext();
    }

    // Sort orders by FT, then by ST (bubble sort for simplicity)
    // Can be optimized to merge sort for better complexity
    for (int i = 0; i < numOrders - 1; i++)
    {
        for (int j = 0; j < numOrders - i - 1; j++)
        {
            Order* o1 = orderArray[j];
            Order* o2 = orderArray[j + 1];
            
            int ft1 = o1->GetFinishTime();
            int ft2 = o2->GetFinishTime();
            int st1 = o1->GetServTime();
            int st2 = o2->GetServTime();
            
            // Sort by FT first, then by ST
            bool shouldSwap = false;
            if (ft1 > ft2)
                shouldSwap = true;
            else if (ft1 == ft2 && st1 > st2)
                shouldSwap = true;
                
            if (shouldSwap)
            {
                Order* temp = orderArray[j];
                orderArray[j] = orderArray[j + 1];
                orderArray[j + 1] = temp;
            }
        }
    }

    // Write header
    outFile << "FT\tID\tAT\tWT\tST\n";

    // Write sorted orders
    for (int i = 0; i < numOrders; i++)
    {
        Order* ord = orderArray[i];
        int ft = ord->GetFinishTime();
        int id = ord->GetID();
        int at = ord->GetArrTime();
        int wt = ord->GetServTime() - ord->GetArrTime();  // WT = ServTime - ArrTime
        int st = ft - ord->GetServTime();                  // ST = FinishTime - ServTime

        outFile << ft << "\t" << id << "\t" << at << "\t" 
                << wt << "\t" << st << "\n";
    }

    delete[] orderArray;

    // Count orders by type
    int normalCount = 0, veganCount = 0, vipCount = 0;
    curr = finished.getHead();
    while (curr)
    {
        ORD_TYPE type = curr->getItem()->GetType();
        if (type == TYPE_NRM) normalCount++;
        else if (type == TYPE_VGAN) veganCount++;
        else if (type == TYPE_VIP) vipCount++;
        curr = curr->getNext();
    }

    // Calculate averages
    double avgWait = (CountFinished > 0) ? (double)TotalWaitTime / CountFinished : 0.0;
    double avgServ = (CountFinished > 0) ? (double)TotalServTime / CountFinished : 0.0;

    // Count total cooks
    int totalCooks = normalCooks.getSize() + veganCooks.getSize() + vipCooks.getSize();

    // Write statistics
    outFile << "\nOrders: " << numOrders 
            << " [Norm:" << normalCount 
            << ", Veg:" << veganCount 
            << ", VIP:" << vipCount << "]\n";
    
    outFile << "Cooks: " << totalCooks
            << " [Norm:" << normalCooks.getSize()
            << ", Veg:" << veganCooks.getSize()
            << ", VIP:" << vipCooks.getSize() << "]\n";
    
    outFile << "Avg Wait = " << fixed << setprecision(2) << avgWait
            << ", Avg Serv = " << avgServ << "\n";
    
    outFile << "Auto-promoted: " << autoPromotedCount << "\n";
    outFile << "Late Orders: " << lateOrderCount << "\n";

    // Per-cook statistics
    // Normal cooks
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook N" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    // Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook G" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    // VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        outFile << "Cook V" << cook->GetID() << ": Orders [Norm:"
                << cook->getNormalOrdersServed() << ", Veg:"
                << cook->getVeganOrdersServed() << ", VIP:"
                << cook->getVIPOrdersServed() << "], Busy: "
                << cook->getTotalBusyTime() << ", Idle: "
                << cook->getTotalIdleTime() << ", Break/Injury: "
                << cook->getTotalBreakTime() << ", Utilization: "
                << fixed << setprecision(1) << cook->getUtilization() << "%\n";
        cookNode = cookNode->getNext();
    }

    outFile.close();
    
    if (pGUI)
        pGUI->PrintMessage("Output file written successfully: " + filename);
}
