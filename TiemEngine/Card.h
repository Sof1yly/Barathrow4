#pragma once
#include <string>
#include <vector>
#include "Action.h"
#include "ImageObject.h"
#include "TextObject.h"

class Card
{
private:
    string name;
    int level;
    string rarityCode;  
    string typeCode;    
    vector<Action*> actions;
    
    // Visual rendering data
    ImageObject* background = nullptr;
    ImageObject* cardFrame = nullptr;
    ImageObject* visual = nullptr;
    ImageObject* typeIcon = nullptr;
    ImageObject* starOverlay = nullptr;
    TextObject* nameText = nullptr;
    bool visualsCreated = false;


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
    
    // Visual rendering methods
    void CreateVisuals();
    void DestroyVisuals();
    bool HasVisuals() const { return visualsCreated; }
    
    std::vector<DrawableObject*> GetAllLayers() const;
    ImageObject* GetBackground() const { return background; }
    ImageObject* GetCardFrame() const { return cardFrame; }
    ImageObject* GetVisual() const { return visual; }
    ImageObject* GetTypeIcon() const { return typeIcon; }
    ImageObject* GetStarOverlay() const { return starOverlay; }
    TextObject* GetNameText() const { return nameText; }
};