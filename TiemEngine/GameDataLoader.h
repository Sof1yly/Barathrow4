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
#include "BuffAction.h"
#include "DebuffAction.h"
#include "EnergyAction.h"
#include "AttackPattern.h"

class GameDataLoader
{
private:
    struct NameAction {
        string name;
        Action* action;
    };

    vector<Card*>      cards;
    vector<NameAction> actions;
    vector<Action*>    actions_list;

    std::unordered_map<std::string, AttackPattern>          patternMap;
    std::unordered_map<const Action*, const AttackPattern*> actionPattern;

    const AttackPattern* findPatternByName(const string& id) const;

    // Factory: creates the correct Action subclass for a given code
    Action* createAction(const std::string& code, int value, float multiplier,const std::string& patternId, Card* card,std::string* outError);

public:
    GameDataLoader();
    ~GameDataLoader();

    // Pattern.txt
    bool loadPatternsFromFile(const std::string& filename,
        std::string* outError = nullptr);

    // CardAction.txt  (Card / Name,Action,Pattern,Level,Rarity,Type,Desc)
    bool loadFromFile(const std::string& filename,std::string* outError = nullptr);

    std::vector<Card*> getCards() const { return cards; }  
    const std::vector<Action*>& getActions() const { return actions_list; }

    const AttackPattern* getPatternForAction(const Action* a) const;

    Card* findEnergyCard() const;
};
