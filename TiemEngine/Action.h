#pragma once
#include <string>
#include <vector>

using namespace std;

enum class type {
	attack,move,energy,buff,deBuff
};
class Action {
private:
	string description;
	int value;
	int damage;
	int move;
	//vector<tile> tiles;
	enum type;
	

public:
	string getDes();
	int getValue();
	float rotation();
	int damage_action();
	int move_action();

};