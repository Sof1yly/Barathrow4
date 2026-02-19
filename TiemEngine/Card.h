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
    string description;
    int level;
    string rarityCode;  
    string typeCode;    
    vector<Action*> actions;

    bool isFast = false;
    bool isTemp = false;
    bool isDeleteAfterUse = false;
    int overclockValue = 0;
    
    // Visual rendering data
    ImageObject* background = nullptr;
    ImageObject* cardFrame = nullptr;
    ImageObject* visual = nullptr;
    ImageObject* typeIcon = nullptr;
    ImageObject* starOverlay = nullptr;
    TextObject* nameText = nullptr;
    TextObject* descriptionText = nullptr;
    bool visualsCreated = false;


public:
    Card(const string& n);
    ~Card();

    const string& getName() const;
    const string& getDescription() const { return description; }
    void setDescription(const string& desc) { description = desc; }
    int getLevel() const { return level; }
    const string& getRarityCode() const { return rarityCode; }
    const string& getTypeCode() const { return typeCode; }
    
    void setLevel(int lv) { level = lv; }
    void setRarityCode(const string& code) { rarityCode = code; }
    void setTypeCode(const string& code) { typeCode = code; }

    bool getIsFast() const { return isFast; }
    void setIsFast(bool v) { isFast = v; }
    bool getIsTemp() const { return isTemp; }
    void setIsTemp(bool v) { isTemp = v; }
    bool getIsDeleteAfterUse() const { return isDeleteAfterUse; }
    void setIsDeleteAfterUse(bool v) { isDeleteAfterUse = v; }
    int getOverclockValue() const { return overclockValue; }
    void setOverclockValue(int v) { overclockValue = v; }
    
    const vector<Action*>& getActions() const;
    void addAction(Action* action);
    void do_action();
    
    // Visual rendering methods
    void CreateVisuals();
    void DestroyVisuals();
    void RefreshDescriptionText();
    bool HasVisuals() const { return visualsCreated; }
    
    vector<DrawableObject*> GetAllLayers() const;
    ImageObject* GetBackground() const { return background; }
    ImageObject* GetCardFrame() const { return cardFrame; }
    ImageObject* GetVisual() const { return visual; }
    ImageObject* GetTypeIcon() const { return typeIcon; }
    ImageObject* GetStarOverlay() const { return starOverlay; }
    TextObject* GetNameText() const { return nameText; }
    TextObject* GetDescriptionText() const { return descriptionText; }
};