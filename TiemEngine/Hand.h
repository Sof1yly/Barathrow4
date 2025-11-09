#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>


#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"

struct CardView
{
    Card* cardData = nullptr;  // which logical card
    ImageObject* image = nullptr;  // its visual
};

class Hand
{
private:
    std::vector<CardView> views;          // all cards in hand
    ImageObject* selectedView = nullptr;  // currently selected (if any)
    ImageObject* hoveredView = nullptr;  // currently hovered (if any)

    std::unordered_map<ImageObject*, glm::vec3> origPos;
    std::unordered_map<ImageObject*, glm::vec2> origSize;
    std::unordered_map<ImageObject*, float>     origRot;

    bool hitTest(ImageObject* v, const glm::vec3& p) const;
    void layoutViews();
    void liftForHover(ImageObject* v);
    void clearHover();

public:
    ImageObject* GetSelectedView() const { return selectedView; }

    // create N visual cards, bound to first N cardData entries
    void CreateVisualHand(int cardCount,
        std::vector<DrawableObject*>& objectsList,
        const std::vector<Card*>& cardData);

    // top-most card under mouse (for hover / click)
    ImageObject* PeekAt(const glm::vec3& mouseWorld);

    // hover highlight handling
    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging);

    // remove card view from hand and re-layout
    void RemoveView(ImageObject* view);

    // find logical card by its image
    Card* FindCardByImage(ImageObject* img);

    // simple select helpers (if you want click-to-select)
    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();
};
