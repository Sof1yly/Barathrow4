#pragma once
#include <string>
#include <vector>
#include "Action.h"

class Card
{
private:
    std::string name;
    int level;
    std::string rarityCode;  
    std::string typeCode;    
    std::vector<Action*> actions;

public:
    Card(const std::string& n);
    ~Card();

    const std::string& getName() const;
    int getLevel() const { return level; }
    const std::string& getRarityCode() const { return rarityCode; }
    const std::string& getTypeCode() const { return typeCode; }
    
    void setLevel(int lv) { level = lv; }
    void setRarityCode(const std::string& code) { rarityCode = code; }
    void setTypeCode(const std::string& code) { typeCode = code; }
    
    const std::vector<Action*>& getActions() const;
    void addAction(Action* action);
    void do_action();
};