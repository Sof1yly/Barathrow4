#pragma once

#include <string>
#include <vector>
#include "Action.h"

class Card
{
private:
    std::string name;
    std::vector<Action*> actions;
    int level;    // 0-3 stars
    int rarity;   // 1-5 rarity (card frame)
    int type;     // 1-5 type (icon at top left)

public:
    explicit Card(const std::string& n) : name(n), level(0), rarity(1), type(1) {}

    const std::string& getName() const;
    const std::vector<Action*>& getActions() const;

    // Getters for visual properties
    int getLevel() const { return level; }
    int getRarity() const { return rarity; }
    int getType() const { return type; }

    // Setters
    void setLevel(int l) { level = l; }
    void setRarity(int r) { rarity = r; }
    void setType(int t) { type = t; }

    void addAction(Action* action);
    void do_action();
};