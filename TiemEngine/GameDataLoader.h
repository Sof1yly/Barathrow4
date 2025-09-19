#pragma once

#include <string>
#include <vector>
#include "Card.h"
#include "Action.h"
#include "AttackAction.h"
#include "MoveAction.h"

using namespace std;

class GameDataLoader {
private:
	struct NameAction {
		string name;
		Action* action;
	};
	vector<Card*> cards;
	vector<NameAction> actions;
	vector<Action*> actions_list;
	


	Action* findActionByName(const string& name)const;

	bool parseActionRow(const vector<string>& cols, string* error);
	bool parseCardRow(const vector<string>& cols, string* error);

public:
	GameDataLoader();
	~GameDataLoader();
	
	bool loadFromFile(const string& filename, string* outError = nullptr);

	const vector<Card*>& getCards() const { return cards; }
	const vector<Action*>& getActions() const { return actions_list; }

};