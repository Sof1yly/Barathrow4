#pragma once

#include <string>
#include <vector>
#include "Card.h"
#include "Action.h"
#include "AttackAction.h"
#include "MoveAction.h"

class GameDataLoader
{
private:
    struct NameAction {
        std::string name;
        Action* action;
    };

    std::vector<Card*>      cards;
    std::vector<NameAction> actions;
    std::vector<Action*>    actions_list;

    Action* findActionByName(const std::string& name) const;
    bool parseActionRow(const std::vector<std::string>& cols, std::string* error);
    bool parseCardRow(const std::vector<std::string>& cols, std::string* error);

public:
    GameDataLoader();
    ~GameDataLoader();

    bool loadFromFile(const std::string& filename, std::string* outError = nullptr);

    const std::vector<Card*>& getCards()   const { return cards; }
    const std::vector<Action*>& getActions() const { return actions_list; }
};
