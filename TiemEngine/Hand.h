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
    ImageObject* selectedView = nullptr;  
    ImageObject* hoveredView = nullptr;  

    std::unordered_map<ImageObject*, glm::vec3> origPos;
    std::unordered_map<ImageObject*, glm::vec2> origSize;
    std::unordered_map<ImageObject*, float>     origRot;

    bool hitTestBase(ImageObject* v, const glm::vec3& p) const;    // use orig fan pos
	bool hitTestCurrent(ImageObject* v, const glm::vec3& p) const;  // use current pos
    void layoutViews();
    void liftForHover(ImageObject* v);
    void clearHover();

public:
    ImageObject* GetSelectedView() const { return selectedView; }


    void CreateVisualHand(int cardCount,
        std::vector<DrawableObject*>& objectsList,
        const std::vector<Card*>& cardData);

    // t (for hover / click)
    ImageObject* PeekAt(const glm::vec3& mouseWorld);

    // hover highlight handling
    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging);

    // remove card view from hand and re-layout
    void RemoveView(ImageObject* view);

    // find logical card by its image
    Card* FindCardByImage(ImageObject* img);

    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();
};
