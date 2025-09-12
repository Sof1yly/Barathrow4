#pragma once
#include "Card.h"
#include "Deck.h"
#include "Hand.h"
#include "Modify.h"

class Player {
private:
	int hp;
	int armor;
	vector<Card>cards;
	Deck deck;
	Hand hand;
	vector<Modify>modifyAttack;

public:
	int getHp()const;
	int getArmor()const;
	const vector<Card>& getCards()const;
	Deck& getDeck();
	Hand& getHand();
	const vector<Modify>& getModifyAttack()const;
	
};