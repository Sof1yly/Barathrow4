#include "Deck.h"

const vector<Card>& Deck::getCards() const
{
	return cards;
}

void Deck::addCard(const Card& card)
{
	cards.push_back(card);
}

void Deck::removeCard(const Card& card)
{
	auto it = find(cards.begin(), cards.end(), card);
	if (it != cards.end()) {
		cards.erase(it);
	}
}
