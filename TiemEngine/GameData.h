#pragma once
#include "GameStateList.h"

class GameData
{
	static GameData* instance;

public :
	GameState gGameStateInit;
	GameState gGameStateCurr;
	GameState gGameStatePrev;
	GameState gGameStateNext;

	// Event effect chosen in EventPage; read by Level at LevelInit
	int  eventEffectType = -1; // cast from EventScene::EffectType; -1 = none

	static GameData* GetInstance();
};

