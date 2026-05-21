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
#include "Boss.h"
#include "Battery.h"
#include "EliteEnemy1.h"
#include "EliteEnemy2.h"
#include "EliteEnemy3.h"
#include "HighlightManager.h"
#include "Player.h"
#include "DeckViewer.h"
#include "CardInspect.h"
#include "CardRewardSystem.h"
#include "ShopSystem.h"
#include "RewardBoxScene.h"
#include "CardActionExecutor.h"
#include "LevelManager.h"
#include "SaveSystem.h"
#include "EventScene.h"
#include "EventRemoveScene.h"
#include "MapScene.h"
#include "SettingPage.h"
#include "PauseMenu.h"

// Floating damage number that drifts upward and fades out
struct DamagePopup {
    TextObject* text = nullptr;
    float timer    = 0.0f;   // elapsed ms
    float duration = 1200.0f; // total lifetime ms
    float floatSpeed = 0.07f; // pixels per ms (upward)
    bool  expired  = false;
};

// Travelling projectile fired by EliteEnemy1
struct Elite1Projectile {
    SpriteObject* sprite   = nullptr;
    glm::vec3     startPos;
    glm::vec3     endPos;
    float         timer    = 0.0f;
    float         duration = 1000.0f; // ms to cross the full grid
    bool          done     = false;
};

// Projectile dropped from the top of the screen by EliteEnemy2 (cross attack)
struct Elite2Projectile {
    SpriteObject* sprite   = nullptr;
    glm::vec3     startPos;
    glm::vec3     endPos;
    float         timer    = 0.0f;
    float         duration = 700.0f; // ms to fall
    bool          done     = false;
};

// Bullet fired by EliteEnemy3 pattern 2 (line charge) in the attack direction
struct Elite3Projectile {
    SpriteObject* sprite   = nullptr;
    glm::vec3     startPos;
    glm::vec3     endPos;
    float         timer    = 0.0f;
    float         duration = 1000.0f;
    bool          done     = false;
};

// Bullet fired by the player when using a range-attack card (PlayerBullet.png)
struct PlayerBullet {
    SpriteObject* sprite   = nullptr;
    glm::vec3     startPos;
    glm::vec3     endPos;
    float         timer    = 0.0f;
    float         duration = 600.0f;
    bool          done     = false;
};

// BossAttack1.png — stationary tile flash for boss patterns 1/2/3
struct BossAttack1Effect {
    SpriteObject* sprite = nullptr;
    bool          done   = false;
};

// BossAttack2.png — travelling projectile for boss patterns 6/7 (falling) and 8 (cross arms)
struct BossAttack2Projectile {
    SpriteObject* sprite   = nullptr;
    glm::vec3     startPos;
    glm::vec3     endPos;
    float         timer    = 0.0f;
    float         duration = 700.0f;
    bool          done     = false;
};

// Summon.png portal (1 row × 6 cols) — plays before the enemy appears on the 4th frame
struct BossSummonPortal {
    SpriteObject*    sprite       = nullptr;
    int              spawnRow     = 0;
    int              spawnCol     = 0;
    Enemy::EnemyType enemyType    = Enemy::EnemyType::A;
    float            timer        = 0.0f;
    float            spawnDelay   = 3 * 300.0f; // 3 frame-delays × 300 ms before frame 4 shows
    bool             enemySpawned = false;
    bool             done         = false;
};

class Level
{
private:
    /////////////
    //Set Boss //
    bool boss = false;
    bool elite1 = false;
    bool elite2 = false;
    bool elite3 = false;
    Boss* bossEnemy = nullptr;
    bool bossActed = false;
    ImageObject* bossHpBg   = nullptr;
    ImageObject* bossHpBar  = nullptr;
    ImageObject* bossHpMask = nullptr;
    TextObject*  bossHpText = nullptr;
    void UpdateBossHPBar();
	/// ///////////

	int turnCount = 0;

    HighlightManager highlightManager;

    TextObject* gameOverText = nullptr;
	TextObject* winText = nullptr;
    bool isGameOver = false;
    float winDelay = 0.0f;
    bool winDelayActive = false;
    int pendingCoinsEarned = 0;
    TextObject* levelNameBanner = nullptr;
    float levelBannerTimer = 0.0f;
    bool levelBannerActive = false;

    // UI HP bar
    ImageObject* hpBar     = nullptr;
    ImageObject* hpMask    = nullptr;
    TextObject*  hpBarText = nullptr;
    TextObject* skipTurnHintText = nullptr;
    TextObject* viewDeckHintText = nullptr;
    TextObject* viewMapHintText  = nullptr;

    GameObject* turnBannerBg = nullptr;
    TextObject* turnBannerText = nullptr;
    int turnBannerState = -1;
    float turnBannerTimer = 0.0f;
    float turnBannerDuration = 2000.0f;

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
    ImageObject* viewMapIcon = nullptr;  // existing Info button used as map shortcut
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

    static constexpr int GRID_ROWS = 9;
    static constexpr int GRID_COLS = 5;

    bool walkable[GRID_ROWS][GRID_COLS];


    int distanceBetweenGridX = 11;
    int distanceBetweenGridY = 21;

 //Set player start position
    const int startRow = 4;
    const int startCol = 3;
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

    RewardBoxScene rewardBoxScene;
    bool inShopOnlyLevel = false;

    LevelManager levelManager;
    TextObject* levelText = nullptr;

    bool fastMode = false;
    ImageObject* speedBtnIcon = nullptr;
    TextObject*  speedBtnText = nullptr;

    // Event scene (shown once at the start of a run)
    EventScene eventScene;
    EventRemoveScene eventRemoveScene;
    bool eventSceneDone = false;

    // Map scene — shown between levels after all rewards are collected
    MapScene mapScene;
    bool mapSceneActive = false;

    // Persistent run effects granted by the event scene
    int  baseHandSize        = 5;
    bool goldBonusActive     = false;
    int  startCombatBarrier  = 0;
    int  startCombatOverclock = 0;

    // Floating damage popups
    std::vector<DamagePopup> damagePopups;

    // Elite1 travelling projectiles
    std::vector<Elite1Projectile> elite1Projectiles;

    // Elite2 falling projectiles
    std::vector<Elite2Projectile> elite2Projectiles;

    // Elite3 directional bullets
    std::vector<Elite3Projectile> elite3Projectiles;

    // Player range-attack bullets
    std::vector<PlayerBullet> playerBullets;

    // Boss attack visuals
    std::vector<BossAttack1Effect>     bossAttack1Effects;
    std::vector<BossAttack2Projectile> bossAttack2Projectiles;
    std::vector<BossSummonPortal>      bossSummonPortals;

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

protected:
    // Shared with subclasses (e.g. MainMenu) so they can open the setting page
    SettingPage settingPage;
    bool settingPageActive = false;

private:
    PauseMenu pauseMenu;
    bool pauseMenuActive     = false;
    bool gameOverScreenActive = false;

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

    void ApplyEventEffect(EventScene::EffectType effect);

    void UpdatePlayerAnimation();



    void PreviewAttackPattern(Card* cardData, int dz);
    void PreviewMovePath(int steps, int dir);


    void PreviewAllEnemyAttacks();

    void PlayerTakeDamage(int damage);
    void HandlePlayerDeath();

	int ConvertDir(PlayerDir dir);

    void UpdateHPBar();

    void EndTurn();

    void AdvanceToNextRound();

    // Spawn a floating damage number at a world position
    void SpawnDamagePopup(glm::vec3 worldPos, int damage);
    void SpawnElite1Projectile(EliteEnemy1* elite1);
    void SpawnElite2Projectile(int centerRow, int centerCol);
    void SpawnElite3Projectile(EliteEnemy3* elite3e);
    void SpawnPlayerBullets(const std::vector<std::pair<int,int>>& targetCells);
    bool IsWalkable(int row, int col) const;

    void SpawnBossSummon();
    void SpawnBatteries();

    // Boss attack visual effects
    void SpawnBossAttack1(const std::vector<std::pair<int,int>>& tiles);
    void SpawnBossAttack2Falling(const std::vector<std::pair<int,int>>& tiles);
    void SpawnBossAttackCross(int centerRow, int centerCol, bool enhanced);

    // Returns true if any Battery is still alive in the enemies list
    bool AnyBatteryAlive() const;

    void SetPlayerSpawnPosition();

    void UpdateBossPlayerPos();
private:
    void InitBossLevel();
    void SpawnEnemiesForLevel();
    void RestoreEnemiesFromSave(const SaveData& sd);
    void LoadEnemyData();
    void ResetForNextCombat();

};
