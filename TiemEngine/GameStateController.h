#pragma once

#include "GameEngine.h"
#include "GameStateList.h"
#include "GameData.h"
#include "Level.h"
#include "LevelTest.h"
#include "Level3.h"
#include "MainMenu.h"
#include "EventPage.h"
#include "TutorialPage.h"

using namespace std;
class GameStateController
{

public:

	
	Level* currentLevel;

	GameStateController();
	void Init(GameState gameStateInit);
	void Update();
};