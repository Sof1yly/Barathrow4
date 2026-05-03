#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "TextObject.h"
#include "Button.h"
#include "AttackPattern.h"
#include "CardSystem.h"
#include "Enemy.h"
#include "HighlightManager.h"
#include "Player.h"
#include "DeckViewer.h"
#include "CardInspect.h"
#include "CardRewardSystem.h"
#include "ShopSystem.h"
#include "CardActionExecutor.h"
#include "LevelManager.h"

// Floating damage number that drifts upward and fades out
struct DamagePopup {
    TextObject* text = nullptr;
    float timer    = 0.0f;   // elapsed ms
    float duration = 1200.0f; // total lifetime ms
    float floatSpeed = 0.07f; // pixels per ms (upward)
    bool  expired  = false;
};

class Level
{
private:
	int turnCount = 0;

    HighlightManager highlightManager;

    TextObject* gameOverText = nullptr;
	TextObject* winText = nullptr;
    bool isGameOver = false;

	int playerHealth = 50;//delete later
    // UI HP bar
    ImageObject* hpBar = nullptr;
    int maxPlayerHealth = 50;
    ImageObject* hpMask = nullptr;
    TextObject* skipTurnHintText = nullptr;
    TextObject* viewDeckHintText = nullptr;

    Player playerData;

    // Render list
    std::vector<DrawableObject*> objectsList;

    // Basic objects
    GameObject* player = nullptr;
    GameObject* testMove = nullptr;
    SpriteObject* playersprite = nullptr; //Real Player
    ImageObject* mainMenu = nullptr;
    Button viewDeckButton;
    Button skipTurnButton;
    ImageObject* testEnemy;
	ImageObject* Background = nullptr;
	std::vector<ImageObject*> gridTiles;
    vector<Enemy*> enemies;


    //////////////////////////////
	// //Enemy state
    int currentEnemyIndex = 0;
    bool enemyActing = false;
    bool enemyPreparingAttack = false;
	bool anyEnemyDied = false;
	// Player state
    // Smooth grid movement
    bool playerMoving = false;
    glm::vec3 playerMoveStart;
    glm::vec3 playerMoveTarget;
    float playerMoveTimer = 0.0f;
    const float PLAYER_MOVE_TIME = 1000.0f; // 1 sec per tile (ms)

    bool playerAttacking = false;
    float attackTimer = 0.0f;
    const float ATTACK_TIME = 800.0f;

    bool pendingAttack = false;

    // Facing / animation
    enum class PlayerDir { DOWN, UP, RIGHT, LEFT };
    enum class PlayerState { IDLE, WALK, ATTACK };

    PlayerDir playerDir = PlayerDir::DOWN;
    PlayerState playerState = PlayerState::IDLE;

	//End player state
    //////////////////////////////

    // Grid
    int   GridStartRow = 0;
    int   GridEndRow = 9;
    int   GridStartCol = 0;
    int   GridEndCol = 5;
    float GridWide = 90.0f;
    float GridHigh = 84.0f;


    int distanceBetweenGridX = 11;
    int distanceBetweenGridY = 21;

 //Set player start position
    const int startRow = 3;
    const int startCol = 2;
    int nowRow = startRow;
    int nowCol = startCol;

    int pendingMoveSteps = 0;
    int pendingMoveZone = -1;
    bool pendingFastCard = false;
    int lagTurns = 0;

    bool playerPlayingOneShot = false;
    bool playerDead = false;
    float playerAnimTimer = 0.0f;
    float playerAnimDuration = 0.0f;
	int direction = 0;//0down 1left 2up 3right


    // Movement
    glm::vec3 testMoveTarget = glm::vec3(0.0f);
    bool testMoveMoving = false;

    // Card system (deck, discard, drag, drop zones, bezier)
    CardSystem cardSystem;

    // Deck viewer for viewing all cards in deck
    DeckViewer deckViewer;

    CardInspect cardInspect;

    CardRewardSystem cardRewardSystem;
    bool rewardPickedAfterWin = false;

    ShopSystem shopSystem;
    bool shopOpenedAfterWin = false;

    LevelManager levelManager;
    bool waitingForRewardToAdvance = false;
    TextObject* levelText = nullptr;

    // Floating damage popups
    std::vector<DamagePopup> damagePopups;

    // Patterns
    std::vector<AttackPattern> patterns;
    AttackPattern rotatedPattern;
    int currentPatternIndex = 0;
    int currentRotation = 0;

	glm::vec3 GridToWorld(int row, int col) const;
    int enemyHighlightIndex = 0;
    //gameloop
    enum class TurnState {
        PLAYER_TURN,
        PLAYER_MOVING,
        ENEMY_TURN,
        END_TURN,
        GAME_OVER
    };

    TurnState turnState = TurnState::PLAYER_TURN;
    bool tempDiscardDone = false;
	//end gameloop

public:
    virtual void LevelLoad();
    virtual void LevelInit();
    virtual void LevelDraw();
    virtual void LevelFree();
    virtual void LevelUpdate();
    virtual void LevelUnload();

    virtual void HandleKey(char key);
    virtual void HandleMouse(int type, int x, int y);

    void ApplyAttackCells(const std::vector<std::pair<IVec2, int>>& cells);
    void ApplyEnemyAttack(Enemy* e);
    bool EnemyCanAttackPlayer(Enemy* e);

    void UpdateTurn();

    void LevelRestart();

    void UpdatePlayerAnimation();

    void SetPlayerIdle(PlayerDir dir);
    void SetPlayerWalk(PlayerDir dir);
	void SetPlayerGetDamage(PlayerDir dir);
    void SetPlayerDie(PlayerDir dir);

    void PreviewAttackPattern(Card* cardData, int dz);
    void PreviewMovePath(int steps, int dir);
	void PreviewEnemyAttack(Enemy* e);

    void PreviewAllEnemyAttacks();

    void PlayerTakeDamage(int damage);
    void HandlePlayerDeath();

	int ConvertDir(PlayerDir dir);

    void UpdateHPBar();

    void EndTurn();

    void AdvanceToNextRound();

    // Spawn a floating damage number at a world position
    void SpawnDamagePopup(glm::vec3 worldPos, int damage);

private:
    void SpawnEnemiesForLevel();

};
