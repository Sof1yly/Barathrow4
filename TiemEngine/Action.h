#pragma once
#include <string>
#include <vector>

using namespace std;

class Deck;

enum class ActionType {
	Attack,Move,Energy,Buff,Debuff
};
class Action {
private:
	Deck* deck;
	int value;
	int rotation;
	int move;
	ActionType type;
	

public:
	Deck* getDeck()const;
	int getValue()const;
	int getRotation()const;
	int getMove()const;
	ActionType getType()const;

	string getDescription();

	int move_action();
	int damage_action();
	

};

class Move_Action :public Action {};

class Damage_Action :public Action {};