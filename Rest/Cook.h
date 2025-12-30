#pragma once

#include "..\Defs.h"

class Cook
{
	int ID;
	COOK_TYPE type;	//for each cook type there is a corresponding type (VIP, Normal, Vegan)
	int speed;		//dishes it can prepare in one clock tick (in one timestep)

public:
	Cook();
	Cook(int id, COOK_TYPE t, int s, int ba = 0, int bd = 0);
	virtual ~Cook();
	int GetID() const;
	COOK_TYPE GetType() const;
	void setID(int);
	void setType(COOK_TYPE);
	void setSpeed(int s) { speed = s; }  // Added setter for speed
	int getSpeed() const { return speed; }  // Added getter
};