#pragma once

#include "Action.h"

using namespace std;

class Card {
private:
	string name;
	vector<Action> actions;


public:
	string getName();

};