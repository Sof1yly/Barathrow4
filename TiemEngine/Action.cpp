#include "Action.h"

int Action::getValue() const
{
	return value;
}

void Action::setValue(int v) {
	value = v;
}

int Action::getBaseValue() const
{
	return baseValue;
}

void Action::setBaseValue(int v) {
	baseValue = v;
}

void Action::resetToBase() {
	value = baseValue;
}

int Action::getRotation() const
{
	return rotation;
}

void Action::setRotation(int r) {
	rotation = r;
}

ActionType Action::getType() const
{
	return type;
}

void Action::setType(ActionType t) {
	type = t;
}

float Action::getMultiplier() const
{
	return multiplier;
}

void Action::setMultiplier(float m) {
	multiplier = m;
}

const string& Action::getActionCode() const
{
	return actionCode;
}

void Action::setActionCode(const string& code) {
	actionCode = code;
}

Action::~Action()
{
}

