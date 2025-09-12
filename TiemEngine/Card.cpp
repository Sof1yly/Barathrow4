#include "Card.h"


const string& Card::getName() const
{
    return name;
}
const vector<Action*>& Card::getActions() const
{
	return actions;
}