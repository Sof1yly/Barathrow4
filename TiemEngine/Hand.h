#pragma once
#include <unordered_map>
#include "Card.h"
#include "DrawableObject.h"
#include "GameObject.h"


class Hand {
private:
	vector<Card*> deck;
	unordered_map<GameObject*, Card*> viewToData;

public:
	//void SetCards(const vector<Card*>& cards);

	void CreateVisualHand(int cardCount, vector<DrawableObject*>& objectsList);

	//Card* GetCardByView(GameObject* view)const;

	int LogicalCount()const { return (int)deck.size(); }
};