#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"

class Hand {
private:
    std::vector<Card*>        deck;
    std::vector<ImageObject*> views;    

    ImageObject* selectedView = nullptr;
    ImageObject* hoveredView = nullptr;

    std::unordered_map<ImageObject*, glm::vec3> origPos;
    std::unordered_map<ImageObject*, glm::vec2> origSize;
    std::unordered_map<ImageObject*, float>     origRot;

    bool hitTest(ImageObject* v, const glm::vec3& p) const;
    void layoutViews();
    void liftForHover(ImageObject* v);
    void clearHover();

public:
    ImageObject* GetSelectedView() const { return selectedView; }

    void CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList);
    ImageObject* PeekAt(const glm::vec3& mouseWorld);
    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging);
    void RemoveView(ImageObject* view);

    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();

    int LogicalCount() const { return (int)deck.size(); }
};
