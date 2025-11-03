#pragma once
#include <unordered_map>
#include "Card.h"
#include "DrawableObject.h"
#include "GameObject.h"


class Hand {
private:
	vector<Card*> deck;
	vector<GameObject*> views;
	//unordered_map<GameObject*, Card*> viewToData;

	GameObject* selectedView = nullptr;

	unordered_map<GameObject*, glm::vec3> origPos;
	unordered_map<GameObject*, glm::vec2> origSize;
	unordered_map<GameObject*, float> origRot;

	bool hitTest(GameObject* v, const glm::vec3& p) const;

	// effects
	void applySelectedVisual(GameObject* v);
	void restoreVisual(GameObject* v);

public:
	GameObject* GetSelectedView() const { return selectedView; }

	void CreateVisualHand(int cardCount, vector<DrawableObject*>& objectsList);

	bool TrySelectAt(const glm::vec3& mouseWorld);
	void Deselect();

	int LogicalCount()const { return (int)deck.size(); }
};