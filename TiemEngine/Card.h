#pragma once

#include "Action.h"

using namespace std;

class Card {
private:
	string name;
	vector<Action*> actions;
	

public:
	const string& getName()const;
	const vector<Action*>& getActions()const;
	void do_action();
};