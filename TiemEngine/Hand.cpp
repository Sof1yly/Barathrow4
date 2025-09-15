#include "Hand.h"

const vector<Card>& Hand::getCards() const
{
	return cards;
}

void Hand::addCard(const Card& card)
{
	cards.push_back(card);
}

void Hand::removeCard(const Card& card)
{
	/*auto it = find(cards.begin(), cards.end(), card);
	if (it != cards.end()) {
		cards.erase(it);
	}*/
}