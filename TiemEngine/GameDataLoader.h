#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

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
    std::vector<NameAction> actions;       // not really used now, but kept
    std::vector<Action*>    actions_list;  // all allocated actions

    // Pattern id -> pattern data
    std::unordered_map<std::string, AttackPattern>          patternMap;
    // Specific Action* (AttackAction) -> pattern to use
    std::unordered_map<const Action*, const AttackPattern*> actionPattern;

    const AttackPattern* findPatternByName(const std::string& id) const;

public:
    GameDataLoader();
    ~GameDataLoader();

    // Pattern.txt
    bool loadPatternsFromFile(const std::string& filename,
        std::string* outError = nullptr);

    // CardAction.txt  (Card / Name,Damage,Move,Pattern)
    bool loadFromFile(const std::string& filename,
        std::string* outError = nullptr);

    const std::vector<Card*>& getCards()   const { return cards; }
    const std::vector<Action*>& getActions() const { return actions_list; }

    const AttackPattern* getPatternForAction(const Action* a) const;
};
