#include "Action.h"


int Action::getValue() const
{
	return value;
}

int Action::getRotation() const
{
	return rotation;
}
ActionType Action::getType() const
{
	return type;
}
string Action::getDescription()
{
	return "";
}



