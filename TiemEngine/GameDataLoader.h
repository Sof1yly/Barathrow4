#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Card.h"
#include "Action.h"
#include "AttackAction.h"
#include "MoveAction.h"
#include "AttackPattern.h"

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

    std::unordered_map<std::string, AttackPattern>          patternMap;
    std::unordered_map<const Action*, const AttackPattern*> actionPattern;

    Action* findActionByName(const std::string& name) const;
    bool parseActionRow(const std::vector<std::string>& cols, std::string* error);
    bool parseCardRow(const std::vector<std::string>& cols, std::string* error);

    const AttackPattern* findPatternByName(const std::string& id) const;

public:
    GameDataLoader();
    ~GameDataLoader();

    bool loadPatternsFromFile(const std::string& filename, std::string* outError = nullptr);

    bool loadFromFile(const std::string& filename, std::string* outError = nullptr);

    const std::vector<Card*>& getCards()   const { return cards; }
    const std::vector<Action*>& getActions() const { return actions_list; }

    const AttackPattern* getPatternForAction(const Action* a) const;
};
