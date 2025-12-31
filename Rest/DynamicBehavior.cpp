#include "Restaurant.h"
// Dynamic Behavior Methods
// ========================================

// Trigger breaks for cooks who have served BO consecutive orders
// Implements overtime/break skipping when system is overloaded
// Complexity: O(C) where C = total number of cooks
void Restaurant::TriggerCookBreaks(int currentTime)
{
    bool overloaded = isSystemOverloaded();
    
    // Check all Normal cooks
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                // OVERTIME: Cook skips break due to overload
                // Penalty: Apply extra fatigue (reduce speed more)
                cook->applyFatigue();  // Double fatigue penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                // Normal break
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check all Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                cook->applyFatigue();  // Overtime penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check all VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->needsBreak() && cook->isAvailable())
        {
            if (overloaded)
            {
                cook->applyFatigue();  // Overtime penalty
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " skipped break (overtime) - extra fatigue applied");
                }
            }
            else
            {
                cook->startBreak(currentTime);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " started break");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
}

// Check if system is overloaded
// Condition: More than 5 VIP orders waiting
// Complexity: O(1)
bool Restaurant::isSystemOverloaded() const
{
    // System is overloaded if there are many waiting VIP orders
    // Threshold: 5 or more VIP orders waiting
    return waitVIP.getSize() >= 5;
}

// Trigger random health emergencies (injuries) for cooks
// Probability: 0.1% per cook per timestep (about 1 injury per 1000 timesteps per cook)
// Recovery duration: 10 timesteps
// Complexity: O(C) where C = total number of cooks
void Restaurant::TriggerRandomInjuries(int currentTime)
{
    // Random injury probability: 0.1% = 0.001
    // Using simple pseudo-random based on current time and cook ID
    
    // Check all cooks for potential injury
    Node<Cook*>* cookNode = normalCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        // Only available or busy cooks can get injured (not already injured/on break)
        if (cook->isAvailable() || cook->isBusy())
        {
            // Pseudo-random: if (time * 7 + id * 13) % 1000 == 0, trigger injury
            // This gives approximately 0.1% chance
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)  // 0.1% chance
            {
                // Injury triggered!
                int recoveryDuration = 10;  // 10 timesteps to recover
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook N" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check Vegan cooks
    cookNode = veganCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->isAvailable() || cook->isBusy())
        {
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)
            {
                int recoveryDuration = 10;
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook G" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
    
    // Check VIP cooks
    cookNode = vipCooks.getHead();
    while (cookNode)
    {
        Cook* cook = cookNode->getItem();
        
        if (cook->isAvailable() || cook->isBusy())
        {
            int randomValue = (currentTime * 7 + cook->GetID() * 13) % 1000;
            
            if (randomValue == 0)
            {
                int recoveryDuration = 10;
                cook->setInjured(currentTime, recoveryDuration);
                
                if (pGUI)
                {
                    pGUI->PrintMessage("Cook V" + to_string(cook->GetID()) + 
                                      " injured! Recovery: " + to_string(recoveryDuration) + " timesteps");
                }
            }
        }
        
        cookNode = cookNode->getNext();
    }
}
