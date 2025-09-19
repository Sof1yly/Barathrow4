#pragma once

#include "Action.h"

using namespace std;

class Card {
private:
	string name;
	vector<Action*> actions;
	

public:
	Card(const string& n) :name(n){}
	const string& getName()const;
	const vector<Action*>& getActions()const;
	void addAction(Action* action);
	void do_action();

};