#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Card.h"
#include "DrawableObject.h"
#include "GameObject.h"

class Hand {
private:
    std::vector<Card*>        deck;
    std::vector<GameObject*>  views;

    GameObject* selectedView = nullptr;

    std::unordered_map<GameObject*, glm::vec3> origPos;
    std::unordered_map<GameObject*, glm::vec2> origSize;
    std::unordered_map<GameObject*, float>     origRot;


    bool hitTest(GameObject* v, const glm::vec3& p) const;

    // recompute fan pattern positions/rotations for all current views
    void layoutViews();

    // effects
    void applySelectedVisual(GameObject* v);
    void restoreVisual(GameObject* v);

public:
    GameObject* GetSelectedView() const { return selectedView; }

    // create initial hand and push GameObjects into objectsList
    void CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList);

    // just return which card is under mouse (no selection, no pop)
    GameObject* PeekAt(const glm::vec3& mouseWorld);

    // remove a view from the hand and re-fan the rest
    void RemoveView(GameObject* view);

    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();

    int LogicalCount() const { return (int)deck.size(); }
};
