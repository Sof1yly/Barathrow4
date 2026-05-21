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

	// Persistent audio / display settings (used by SettingPage)
	int  musicVolume     = 8;     // 0-10
	bool musicEnabled    = true;
	int  soundVolume     = 8;     // 0-10
	bool soundEnabled    = true;
	int  resolutionIndex = 2;     // 0=1920x1080  1=1600x900  2=1280x720

	static GameData* GetInstance();
};

