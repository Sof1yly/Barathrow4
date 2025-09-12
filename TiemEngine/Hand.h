#pragma once

#include "Card.h"

class Hand {
private:
	vector<Card> cards;
public:
	const vector<Card>& getCards()const;
	void addCard(const Card& card);
	void removeCard(const Card& card);

};