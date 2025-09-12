#pragma once
#include <vector>
#include "Tile.h"

using namespace std;

class Board {
private:
	vector<vector<Tile>> tiles;
public:

	const vector<vector<Tile>>& getTiles()const;



	
};