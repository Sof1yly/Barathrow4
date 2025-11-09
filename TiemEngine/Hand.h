#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "Card.h"
#include "DrawableObject.h"
#include "GameObject.h"

class Hand {
private:
    std::vector<Card*>       deck;     // optional logical deck
    std::vector<GameObject*> views;    // card GameObjects in the hand

    GameObject* selectedView = nullptr; // if you use selection
    GameObject* hoveredView = nullptr; // currently hovered (previewed) card

    // store base fan transform for each view
    std::unordered_map<GameObject*, glm::vec3> origPos;
    std::unordered_map<GameObject*, glm::vec2> origSize;
    std::unordered_map<GameObject*, float>     origRot;

    // -------- internal helpers --------
    bool hitTest(GameObject* v, const glm::vec3& p) const;

    // arrange all current views in fan pattern + update origPos/origSize/origRot
    void layoutViews();

    // hover visuals
    void liftForHover(GameObject* v);
    void clearHover();

public:
    GameObject* GetSelectedView() const { return selectedView; }

    // create visual cards and push into objectsList
    void CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList);

    // topmost card under mouse, no side-effects
    GameObject* PeekAt(const glm::vec3& mouseWorld);

    // called every mouse move to manage hover preview
    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging);

    // when a card is consumed: remove from structures & refan
    void RemoveView(GameObject* view);

    // optional existing APIs, keep them if used elsewhere
    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();

    int LogicalCount() const { return (int)deck.size(); }
};
