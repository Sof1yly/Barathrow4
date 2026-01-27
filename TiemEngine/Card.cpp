#include "Card.h"

Card::Card(const std::string& n)
    : name(n), level(0), rarityCode("sta"), typeCode("atk")  
{
}

Card::~Card()
{
    for (Action* a : actions) {
        delete a;
    }
    actions.clear();  
}

const std::string& Card::getName() const
{
    return name;
}

const std::vector<Action*>& Card::getActions() const
{
    return actions;
}

void Card::addAction(Action* action)
{
    if (action) {
        actions.push_back(action);
    }
}

void Card::do_action()
{
    for (Action* action : actions)
        action->do_action();
}
