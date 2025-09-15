#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;



enum class ActionType {
	Attack,Move,Energy,Buff,Debuff
};
class Action {
private:
	int value;
	int rotation;
	ActionType type;
	

public:

	int getValue()const;
	void setValue(int v);
	int getRotation()const;
	ActionType getType()const;

	string getDescription();

	virtual void do_action() = 0;
	
	

};

