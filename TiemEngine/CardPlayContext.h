#pragma once

#include <vector>
#include "AttackPattern.h"

// Forward declarations - avoid heavy includes
class Player;
class Enemy;
class CardSystem;
class DrawableObject;

// All the game state that card actions need to read/write
struct CardPlayContext {
    Player& player;
    std::vector<Enemy*>& enemies;
    CardSystem& cardSystem;
    std::vector<DrawableObject*>& objectsList;

    int dropZone;       // 0=LEFT, 1=UP, 2=DOWN, 3=RIGHT
    int playerRow;
    int playerCol;

    // Grid bounds
    int gridStartRow;
    int gridEndRow;
    int gridStartCol;
    int gridEndCol;
};
