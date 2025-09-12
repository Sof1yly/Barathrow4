#include "Action.h"

Deck* Action::getDeck() const
{
	return deck;
}

int Action::getValue() const
{
	return value;
}

int Action::getRotation() const
{
	return rotation;
}
int Action::getMove() const
{
	return move;
}
ActionType Action::getType() const
{
	return type;
}
string Action::getDescription()
{
	return "";
}
int Action::move_action()
{
	return 0;
}
int Action::damage_action()
{
	return 0;
}
