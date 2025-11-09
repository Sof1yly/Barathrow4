#pragma once

#include <string>
#include <vector>
#include "Action.h"

class Card
{
private:
    std::string name;
    std::vector<Action*> actions;

public:
    explicit Card(const std::string& n) : name(n) {}

    const std::string& getName() const;
    const std::vector<Action*>& getActions() const;

    void addAction(Action* action);
    void do_action();
};
