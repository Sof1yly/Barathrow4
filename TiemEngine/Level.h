#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "Button.h"
#include "AttackPattern.h"
#include "GameDataLoader.h"
#include "Hand.h"
#include "Enemy.h"

class Level
{
private:

	int playerHealth = 5;//delete later
    // Render list
    std::vector<DrawableObject*> objectsList;

    // Basic objects
    GameObject* player = nullptr;
    GameObject* testMove = nullptr;
    SpriteObject* playersprite = nullptr; //Real Player
    ImageObject* mainMenu = nullptr;
    ImageObject* testEnemy;
    Enemy* enemy;


    //////////////////////////////
	// Player state
    // Smooth grid movement
    bool playerMoving = false;
    glm::vec3 playerMoveStart;
    glm::vec3 playerMoveTarget;
    float playerMoveTimer = 0.0f;
    const float PLAYER_MOVE_TIME = 1000.0f; // 1 sec per tile (ms)

    // Facing / animation
    enum class PlayerDir { DOWN, UP, RIGHT, LEFT };
    PlayerDir playerDir = PlayerDir::DOWN;
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

    int nowRow = 0;
    int nowCol = 0;

    // Movement
    glm::vec3 testMoveTarget = glm::vec3(0.0f);
    bool      testMoveMoving = false;

    // Data & hand
    GameDataLoader dataLoader;
    Hand hand;

    //piles
    std::vector<Card*> deck; //view deck
    std::vector<Card*> discard; // re-draw

    //Deck Ui
	ImageObject* viewDeckButton = nullptr;
	ImageObject* reDrawButton = nullptr;

    // Drop zones
    GameObject* dropZones[4] = { nullptr, nullptr, nullptr, nullptr };
    bool        dropZonesCreated = false;
    bool        dropZonesVisible = false;
    glm::vec3   dropZoneSavedPos[4];

    // Dragging cards
    bool         isDragging = false;
    bool         isHolding = false;
    ImageObject* draggingCard = nullptr;
    ImageObject* pendingCard = nullptr;
    glm::vec3    dragStartPos = glm::vec3(0.0f);
    glm::vec3    dragMouseWorld = glm::vec3(0.0f);
    glm::vec3    dragAnchor = glm::vec3(0.0f);

    // Bezier leash
    static const int BEZIER_SEGMENTS = 32;
    std::vector<GameObject*> bezierSegments;
    bool  bezierCreated = false;
    float screenCenterY = 0.0f;

    // Patterns
    std::vector<AttackPattern> patterns;
    AttackPattern rotatedPattern;
    int currentPatternIndex = 0;
    int currentRotation = 0;

	glm::vec3 GridToWorld(int row, int col) const;

    //gameloop
    enum class TurnState {
        PLAYER_TURN,
        ENEMY_TURN
    };

    TurnState turnState = TurnState::PLAYER_TURN;
	//end gameloop


    // helpers
    void CreateDropZones(std::vector<DrawableObject*>& list);
    void ShowDropZones();
    void HideDropZones();

    void EnsureBezierSegments(std::vector<DrawableObject*>& list);
    void HideBezier();
    void UpdateBezier(const glm::vec3& P0, const glm::vec3& P1);

    bool IsPointInsideZone(const glm::vec3& p, DrawableObject* zone) const;
    int  HitDropZone(const glm::vec3& p) const;

    void BeginDrag(ImageObject* card, const glm::vec3& mouseWorld);
    void UpdateDrag(const glm::vec3& mouseWorld);
    void EndDrag(const glm::vec3& mouseWorld);

    void ShuffleDeck();
    void DealNewHand(int cardCount);

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
    void ApplyEnemyAttack();
    void MoveEnemyTowardPlayer();
    bool EnemyCanAttackPlayer();

    void UpdateTurn();

    void LevelRestart();

    void SetPlayerIdle(PlayerDir dir);
    void SetPlayerWalk(PlayerDir dir);

};
