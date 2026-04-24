#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;

// Forward declarations for execute()
struct CardPlayContext;
struct CardPlayResult;

enum class ActionType {
	Attack, Move, Debuff, Buff, Energy
};

class Action {
private:
	int value;
	int baseValue;
   int repeatCount = 1;
	int rotation;
	ActionType type;
	float multiplier;
	string actionCode;

public:
	virtual ~Action();

	int getValue() const;
	void setValue(int v);
	int getBaseValue() const;
	void setBaseValue(int v);
	void resetToBase();
	int getRotation() const;
	void setRotation(int r);
	ActionType getType() const;
	void setType(ActionType t);
	float getMultiplier() const;
	void setMultiplier(float m);
    int getRepeatCount() const;
	void setRepeatCount(int c);
	const string& getActionCode() const;
	void setActionCode(const string& code);

	// Each action subclass implements its own execution logic
	virtual void execute(CardPlayContext& ctx, CardPlayResult& result) = 0;

	virtual void do_action() = 0;
};

