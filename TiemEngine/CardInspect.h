#pragma once

#include <vector>
#include <string>

#include "Card.h"
#include "CardSystem.h"

class CardInspect : public Card
{
private:
    bool visible = false;
    Card* inspectedCard = nullptr;
    std::vector<DrawableObject*> inspectObjects;

    std::string GetKeywordTitle(const Action* action, const std::string& fallbackCode) const;
    std::string BuildInspectText(Card* cardData, const CardSystem& cardSystem) const;
    void BuildInspectGrid(Card* cardData, const CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList);

public:
    CardInspect();
    ~CardInspect();

    bool IsVisible() const { return visible; }
    bool IsInspecting(const Card* cardData) const { return visible && inspectedCard == cardData; }

    void Show(Card* cardData, CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList);
    void Hide(std::vector<DrawableObject*>& objectsList);
};
