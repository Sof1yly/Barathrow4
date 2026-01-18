#include "GameStateController.h"

GameStateController::GameStateController()
{
	// set the initial game state
	GameData::GetInstance()->gGameStateInit =
	GameData::GetInstance()->gGameStateCurr =
	GameData::GetInstance()->gGameStatePrev =
	GameData::GetInstance()->gGameStateNext = GameState::GS_NONE;
}

void GameStateController::Init(GameState gameStateInit)
{
	// set the initial game state
	GameData::GetInstance()->gGameStateInit = gameStateInit;

	// reset the current, previoud and next game
	GameData::GetInstance()->gGameStateCurr =
	GameData::GetInstance()->gGameStatePrev =
	GameData::GetInstance()->gGameStateNext = GameData::GetInstance()->gGameStateInit;

	// call the update to set up level class
	Update();
}

void GameStateController::Update()
{
	// Handle state transitions
	if (GameData::GetInstance()->gGameStateNext != GameData::GetInstance()->gGameStateCurr)
	{
		GameState nextState = GameData::GetInstance()->gGameStateNext;

		// Handle restart
		if (nextState == GameState::GS_RESTART)
		{
			if (currentLevel)
			{
				currentLevel->LevelRestart();
			}
			// Reset to current state (stay in same level)
			GameData::GetInstance()->gGameStateNext = GameData::GetInstance()->gGameStateCurr;
			return;
		}

		// Handle quit
		if (nextState == GameState::GS_QUIT)
		{
			return;
		}

		// Clean up current level before switching
		if (currentLevel)
		{
			currentLevel->LevelFree();
			currentLevel->LevelUnload();
			delete currentLevel;
			currentLevel = nullptr;
		}

		// Update states
		GameData::GetInstance()->gGameStatePrev = GameData::GetInstance()->gGameStateCurr;
		GameData::GetInstance()->gGameStateCurr = nextState;

		// Initialize new level
		switch (GameData::GetInstance()->gGameStateCurr)
		{
		case GameState::GS_LEVEL1:
			currentLevel = new Level();
			currentLevel->LevelLoad();
			currentLevel->LevelInit();
			break;
		case GameState::GS_LEVEL2:
			currentLevel = new LevelTest();
			currentLevel->LevelLoad();
			currentLevel->LevelInit();
			break;
		case GameState::GS_LEVEL3:
			currentLevel = new Level3();
			currentLevel->LevelLoad();
			currentLevel->LevelInit();
			break;
		default:
			cout << "gGameStateCurr : invalid state!!" << endl;
			exit(1);
		}
	}
	else
	{
		// First time initialization
		if (!currentLevel)
		{
			switch (GameData::GetInstance()->gGameStateCurr)
			{
			case GameState::GS_LEVEL1:
				currentLevel = new Level();
				currentLevel->LevelLoad();
				currentLevel->LevelInit();
				break;
			case GameState::GS_LEVEL2:
				currentLevel = new LevelTest();
				currentLevel->LevelLoad();
				currentLevel->LevelInit();
				break;
			case GameState::GS_LEVEL3:
				currentLevel = new Level3();
				currentLevel->LevelLoad();
				currentLevel->LevelInit();
				break;
			default:
				cout << "gGameStateCurr : invalid state!!" << endl;
				exit(1);
			}
		}
	}
}