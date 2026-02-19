#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;

enum class ActionType {
	Attack, Move, Energy, Buff, Debuff
};

class Action {
private:
	int value;
	int rotation;
	ActionType type;
	float multiplier;
	string actionCode;

public:
	virtual ~Action();

	int getValue() const;
	void setValue(int v);
	int getRotation() const;
	void setRotation(int r);
	ActionType getType() const;
	void setType(ActionType t);
	float getMultiplier() const;
	void setMultiplier(float m);
	const string& getActionCode() const;
	void setActionCode(const string& code);

	virtual void do_action() = 0;
};

