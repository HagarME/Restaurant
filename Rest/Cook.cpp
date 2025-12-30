#include "Cook.h"



Cook::Cook()
	: ID(0), type(COOK_NRM), speed(1)
{
}

Cook::~Cook()
{
}

int Cook::GetID() const
{
	return ID;
}

COOK_TYPE Cook::GetType() const
{
	return type;
}

void Cook::setID(int id)
{
	ID = id;
}

void Cook::setType(COOK_TYPE t)
{
	type = t;
}