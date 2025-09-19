#include "GameDataLoader.h"
#include <fstream>
#include <sstream>


GameDataLoader::GameDataLoader() = default;

GameDataLoader::~GameDataLoader() {
	for (Card* c : cards) {
		delete c;
	}
	cards.clear();

	for (Action * a : actions_list) {
		delete a;
	}
	actions_list.clear();
	actions.clear();
}

Action* GameDataLoader::findActionByName(const string& name)const {
	for (const NameAction& na : actions) {
		if (na.name == name) {
			return na.action;
		}
	}
	return nullptr;
}

//row cols = { action name, damage, move}
bool GameDataLoader::parseActionRow(const vector<string>& cols, string* error) {
	if (cols.size() < 3) {
		if (error) {
			cout << "Not enough columns for action" << endl;
			return false;
		}
	}

	const string& name = cols[0];
	const string& sDmg = cols[1];
	const string& sMove = cols[3];

	if (name.empty()) {
		if (error) {
			cout << "action name is empty" << endl;
			return false;
		}
	}

	if (findActionByName(name) != nullptr) {
		if (error) {
			cout << "action name is duplicated: " << name << endl;
			return false;
		}
	}

	int damage = 0;
	int move = 0;

	if (!sDmg.empty()) {
		try {
			damage = stoi(sDmg);
		}
		catch (const exception& e) {
			if (error) {
				cout << "invalid damage value: " << sDmg << endl;
				return false;
			}
		}
	}
	if (!sMove.empty()) {
		try 
		{
			move = stoi(sMove);
		}
		catch (const exception& e) {
			if (error) {
				cout << "invalid move value: " << sMove << endl;
				return false;
			}
		}
	}

	if (damage > 0 && move > 0) {
		if (error) {
			cout << "action cannot have both damage and move: " << name << endl;
			return false;
		}
	}
	if (damage <= 0 && move <= 0) {
		if (error) {
			cout << "action must have either damage or move: " << name << endl;
			return false;
		}
	}	

	Action* a = nullptr;

	if (damage > 0) {
		auto* aa = new AttackAction();
		aa->setValue(damage);
		a = aa;
	}
	else {
		auto* ma = new MoveAction();
		ma->setValue(move);
		a = ma;
	}

	actions.push_back({ name, a });
	actions_list.push_back(a);
	return true;

}

bool GameDataLoader::parseCardRow(const vector<string>& cols, string* error) {
	if (cols.empty() || cols[0].empty()) {
		if (error) {
			cout << "Card name is empty" << endl;
			return false;
		}
	}

	const string& cardName = cols[0];
	Card* c = new Card(cardName);

	for (size_t i = 1; i < cols.size(); i++) {
		const string& actionName = cols[i];
		if (actionName.empty()) {
			continue;
		}
		Action* a = findActionByName(actionName);
		if (a == nullptr) {
			if (error) {
				cout << "Action not found for card: " << actionName << endl;
				delete c;
				return false;
			}
		}
		c->addAction(a);
	}
	cards.push_back(c);
	return true;
}


bool GameDataLoader::loadFromFile(const string& filename, string* outError) {
	for (Card* c : cards) { delete c; } cards.clear();
	for (Action* a : actions_list) { delete a; } actions_list.clear(); actions.clear();


	ifstream file(filename);
	if (!file.is_open()) {
		if (outError) {
			*outError = "Failed to open file: " + filename;
		}
		return false;
	}
	enum Section { NONE, ACTIONS, CARDS };
	Section currentSection = NONE;

	string line;
	while (getline(file, line)) {
		if (line.empty()) continue;
		if (line[0] == '#') continue;
		if (line.size() > 1 && line[0] == '/' && line[1] == '/') continue;

		if (line == "[Action]") {
			currentSection = ACTIONS;
			continue;
		}
		else if (line == "[Card]") {
			currentSection = CARDS;
			continue;
		}

		if (line.rfind("Name", 0) == 0) continue; //skip header

		vector<string>cols;
		string token;
		stringstream ss(line);

		while (getline(ss, token, ',')) {
			cols.push_back(token);
		}
		bool ok = false;
		if (currentSection == ACTIONS) {
			ok = parseActionRow(cols, outError);
		}
		else if (currentSection == CARDS) {
			ok = parseCardRow(cols, outError);
		}
		else {
			if (outError) {
				cout << "Data outside of section: " << line << endl;
			}
			ok = false;
		}
		if (!ok)
		{
			return false;
		}
		return true;
	}
}

