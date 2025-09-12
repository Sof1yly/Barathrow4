#include "Player.h"

int Player::getHp() const
{
	return hp;
}

int Player::getArmor() const
{
	return armor;
}

const vector<Card>& Player::getCards() const
{
	return cards;
}

Deck& Player::getDeck()
{
	return deck;
}

Hand& Player::getHand()
{
	return hand;
}

const vector<Modify>& Player::getModifyAttack() const
{
	return modifyAttack;
}
