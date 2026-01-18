#pragma once

enum GameState
{
	// list of all game states 
	GS_LEVEL1 = 0,
	GS_LEVEL2 = 1,
	GS_LEVEL3 = 2,

	// special game state. Do not change
	GS_RESTART,
	GS_QUIT,
	GS_NONE
};