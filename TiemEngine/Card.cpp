#include "Card.h"

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
    actions.push_back(action);
}

void Card::do_action()
{
    for (Action* action : actions)
        action->do_action();
}
