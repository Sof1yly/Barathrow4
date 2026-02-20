#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "TriangleMeshVbo.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "CombineObject.h"
#include "TextObject.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

// Lifecycle
void Level::LevelLoad()
{
    auto* square = new SquareMeshVbo();
    square->LoadData();
    GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);

    auto* sprite = new SpriteMeshVbo();
    sprite->LoadData();
    GameEngine::GetInstance()->AddMesh(SpriteMeshVbo::MESH_NAME, sprite);

    auto* triangle = new TriangleMeshVbo();
    triangle->LoadData();
    GameEngine::GetInstance()->AddMesh(TriangleMeshVbo::MESH_NAME, triangle);

    cout << "Load Level 00" << endl;
}

void Level::LevelInit()
{
	ImageObject* Background = new ImageObject();
	Background->SetSize(1920.0f, -1080.0f);
	Background->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	Background->SetTexture("../Resource/Texture/BG/Floor1_FHD.PNG");
	objectsList.push_back(Background);

	// Initialize player state
    playerDir = PlayerDir::DOWN;
    playerState = PlayerState::IDLE;
    UpdatePlayerAnimation();
    maxPlayerHealth = playerHealth;

    // 1) Tile grid (your original)
    for (int i = GridStartRow; i < GridEndRow; ++i) {
        for (int j = GridStartCol; j < GridEndCol; ++j) {
            ImageObject* tile = new ImageObject();
            tile->SetTexture("../Resource/Texture/BG/F1Grid.png");
            tile->SetSize(GridWide, GridHigh);
            tile->SetPosition(glm::vec3(
                i * 101.0f - 404.0f,
                j * -105.0f + 352.0f,
                0.0f
            ));
            objectsList.push_back(tile);
        }
    }

    AttackHighlights(objectsList);
    HideAttackHighlights();

    //Enemy
	enemy = new Enemy();
    enemy->setNowPosition(8, 0); //row=8,col=0

	ImageObject* enemyObj = new ImageObject();
	enemyObj->SetSize(100.0f, -100.0f);
    enemyObj->SetTexture("../Resource/Texture/Enemy/Enemy1.png");

	//glm::vec3 pos = GridToWorld(enemy->getNowRow(), enemy->getNowCol()); //  glm::vec3 pos = GridToWorld(8, 0);
    glm::vec3 pos = GridToWorld(8, 0);
	enemyObj->SetPosition(pos);

	objectsList.push_back(enemyObj); 
    enemy->setObject(enemyObj);
    enemy->UpdateTextPosition();
    objectsList.push_back(enemy->getHPText());


    // 3) Player sprite (3x4, 192x256)
    {
        SpriteObject* playerSprite =
            new SpriteObject("../Resource/Texture/Player_sprite2.png", 6,16);

        playerSprite->SetSize(200.0f, -200.0f);

        glm::vec3 startPos = GridToWorld(0, 0);
        playerSprite->SetPosition(startPos);

        playerSprite->SetAnimationLoop(
            0,      // start frame
            0,      // row
            2,      // end frame
            800     // ms per frame
        );
        playerSprite->NextAnimation();

        objectsList.push_back(playerSprite);
        playersprite = playerSprite;
    }

    // 4) Demo objects
    {
        /*GameObject* obj = new GameObject();
        obj->SetColor(1.0f, 0.0f, 0.0f);
        obj->SetSize(200.0f, 200.0f);
        objectsList.push_back(obj);
        player = obj;*/
    }

    {
        GameObject* obj2 = new GameObject();
        obj2->SetColor(0.0f, 1.0f, 0.0f);
        obj2->SetSize(50.0f, 50.0f);
        obj2->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
        objectsList.push_back(obj2);
    }

    {
        /*GameObject* obj3 = new GameObject();
        obj3->SetColor(0.0f, 0.0f, 1.0f);
        obj3->SetSize(100.0f, 100.0f);
        obj3->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        objectsList.push_back(obj3);
        testMove = obj3;*/
    }

    //testMoveTarget = testMove->GetPosition();
    //testMoveMoving = false;


    //  Menu button + UI
    {
        /*ImageObject* img = new ImageObject();
        img->SetSize(50.0f, -50.0f);
        img->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
        img->SetTexture("../Resource/Texture/menu.png");
        objectsList.push_back(img);*/
    }

    {
        ImageObject* HPbg = new ImageObject();
        HPbg->SetSize(300.0f, -80.0f);
        HPbg->SetPosition(glm::vec3(-800.0f, 500.0f, 0.0f));
        HPbg->SetTexture("../Resource/Texture/UI/Blank_HPbar.PNG");
        objectsList.push_back(HPbg);
    }

    {
        hpBar = new ImageObject();
        hpBar->SetSize(300.0f, -80.0f);
        hpBar->SetPosition(glm::vec3(-800.0f, 500.0f, 0.0f));
        hpBar->SetTexture("../Resource/Texture/UI/HPbar.PNG");
        objectsList.push_back(hpBar);
    }

    {
        hpMask = new ImageObject();
        hpMask->SetSize(0.0f, -80.0f);  // starts hidden
        hpMask->SetPosition(glm::vec3(-800.0f, 500.0f, 5.0f));
        hpMask->SetTexture("../Resource/Texture/UI/HPbarmask.png");
        objectsList.push_back(hpMask);
    }

    {
        ImageObject* Shieldbg = new ImageObject();
        Shieldbg->SetSize(300.0f, -80.0f);
        Shieldbg->SetPosition(glm::vec3(-450.0f, 10000.0f, 0.0f)); //make it hidden first//real pos -450.0,500.0
        Shieldbg->SetTexture("../Resource/Texture/UI/Blank_Shieldbar.PNG");
        objectsList.push_back(Shieldbg);
    }

    {
        ImageObject* Setting = new ImageObject();
        Setting->SetSize(80.0f, -80.0f);
        Setting->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
        Setting->SetTexture("../Resource/Texture/UI/Setting.PNG");
        objectsList.push_back(Setting);
    }

    {
        ImageObject* Info = new ImageObject();
        Info->SetSize(80.0f, -80.0f);
        Info->SetPosition(glm::vec3(805.0f, 500.0f, 0.0f));
        Info->SetTexture("../Resource/Texture/UI/Info.PNG");
        objectsList.push_back(Info);
    }

    {
        ImageObject* ViewDeck = new ImageObject();
        ViewDeck->SetSize(80.0f, -80.0f);
        ViewDeck->SetPosition(glm::vec3(710.0f, 500.0f, 0.0f));
        ViewDeck->SetTexture("../Resource/Texture/UI/ViewDeck.PNG");
        objectsList.push_back(ViewDeck);
    }

    {
        ImageObject* menu = new ImageObject();
        menu->SetSize(1000.0f, -400.0f);
        menu->SetPosition(glm::vec3(0.0f, 10000.0f, 0.0f)); // hidden
        menu->SetTexture("../Resource/Texture/MainMenu.png");
        objectsList.push_back(menu);
        mainMenu = menu;
    }

    std::string error;

    // 1) Load pattern shapes
    // 2) Load actions + cards (with Pattern column)
    if (!cardSystem.LoadData("../Resource/GameData/Pattern.txt","../Resource/GameData/CardAction.txt", &error)) {
        std::cerr << "Error loading card data: " << error << std::endl;
    }

    cardSystem.ShuffleDeck();

    // Start the game with 5 cards in hand
    cardSystem.DealNewHand(5, objectsList);

    // Card system UI (discard/draw pile buttons + drop zones)
    cardSystem.InitUI(objectsList);

	////////////////////////////////////////test sprite *delete later
 //   SpriteObject* sprite = new SpriteObject("../Resource/Texture/TestSprite.png", 4, 7);
 //   sprite->SetSize(50.0f, -50.0f);
 //   sprite->SetPosition(glm::vec3(800.0f, 0.0f, 0.0f));
 //   sprite->SetAnimationLoop(0, 0, 27, 50);
 //   sprite->NextAnimation();
 //   objectsList.push_back(sprite);
	////////////////////////////////////////test sprite *delete later


    //TextObject* text = new TextObject();
    //text->SetPosition((glm::vec3(500.0f, 200.0f, 0.0f)));
    //SDL_Color color = {0, 255,0 };
    //text->LoadText("hello world!", color, 20);
    //objectsList.push_back(text);

    std::cout << "Init Level" << std::endl;
}

void Level::LevelUpdate()
{
    int deltaTime = GameEngine::GetInstance()->GetDeltaTime();

    UpdateTurn();
    if (enemy)
    {
        enemy->UpdateTextPosition();
        enemy->Update(deltaTime / 1000.0f);
    }
    for (auto* obj : objectsList)
        obj->Update((float)deltaTime);

    if (playerAttacking)
    {
        attackTimer += deltaTime;

        if (attackTimer >= ATTACK_TIME)
        {
            playerAttacking = false;
            pendingAttack = false;

            std::cout << "[Attack Animation Finished]\n";
            if (pendingMoveSteps > 0)
            {
                playerState = PlayerState::WALK;
                UpdatePlayerAnimation();
            }
            else
            {
                playerState = PlayerState::IDLE;
                UpdatePlayerAnimation();

                if (pendingFastCard)
                {
                    std::cout << "[Fast Card] Returning to PLAYER_TURN.\n";
                    turnState = TurnState::PLAYER_TURN;
                    pendingFastCard = false;
                }
                else
                {
                    turnState = TurnState::ENEMY_TURN;
                }
            }
        }

        return;
    }

    if (turnState == TurnState::PLAYER_MOVING && pendingMoveSteps > 0 && !playerMoving && !playerAttacking)
    {
        int targetRow = nowRow;
        int targetCol = nowCol;

        switch (pendingMoveZone)
        {
        case 0: if (nowRow > GridStartRow) targetRow--; break;
        case 3: if (nowRow < GridEndRow - 1) targetRow++; break;
        case 1: if (nowCol > GridStartCol) targetCol--; break;
        case 2: if (nowCol < GridEndCol - 1) targetCol++; break;
        }

        if (enemy &&
            enemy->getNowRow() == targetRow &&
            enemy->getNowCol() == targetCol)
        {
            std::cout << "[Move Blocked] Enemy blocks the tile ("
                << targetRow << "," << targetCol << ")\n";

            pendingMoveSteps = 0;
            playerMoving = false;

            playerState = PlayerState::IDLE;
            UpdatePlayerAnimation();

            if (pendingFastCard)
            {
                turnState = TurnState::PLAYER_TURN;
                pendingFastCard = false;
            }
            else
            {
                turnState = TurnState::ENEMY_TURN;
            }
            return;
        }

        if (targetRow == nowRow && targetCol == nowCol)
        {
            std::cout << "[Card Move Blocked] Edge reached.\n";

            pendingMoveSteps = 0;
            playerMoving = false;

            playerState = PlayerState::IDLE;
            UpdatePlayerAnimation();

            if (pendingFastCard)
            {
                turnState = TurnState::PLAYER_TURN;
                pendingFastCard = false;
            }
            else
            {
                turnState = TurnState::ENEMY_TURN;
            }
            return;
        }


        playerMoving = true;
        HideMoveHighlights();
        playerMoveTimer = 0.0f;

        playerMoveStart = playersprite->GetPosition();
        playerMoveTarget = GridToWorld(targetRow, targetCol);

        nowRow = targetRow;
        nowCol = targetCol;

        pendingMoveSteps--;
    }



    if (enemy && enemy->getHealth() <= 0)
    {
        ImageObject* obj = enemy->getObject();
        if (obj)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end())
                objectsList.erase(it);

            delete obj;
            enemy->setObject(nullptr);
        }

        TextObject* hp = enemy->getHPText();
        if (hp)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), hp);
            if (it != objectsList.end())
                objectsList.erase(it);
            // Do NOT delete hp here — Enemy destructor owns and frees it.
        }
        delete enemy;  // Enemy::~Enemy() deletes hpText
        enemy = nullptr;
    }


    // Smooth move testMove object
    /*if (testMoveMoving && testMove) {
        glm::vec3 cur = testMove->GetPosition();
        glm::vec3 diff = testMoveTarget - cur;
        float dist = glm::length(diff);

        if (dist > 1.0f) {
            glm::vec3 dir = glm::normalize(diff);
            float speed = 0.5f * deltaTime;
            testMove->Translate(dir * speed);
        }
        else {
            testMove->SetPosition(testMoveTarget);
            testMoveMoving = false;
        }
    }*/

    if (playerMoving && playersprite)
    {
        playerMoveTimer += deltaTime;
        float t = playerMoveTimer / PLAYER_MOVE_TIME;
        t = std::min(t, 1.0f);

        glm::vec3 newPos =
            playerMoveStart + (playerMoveTarget - playerMoveStart) * t;

        playersprite->SetPosition(newPos);

        if (t >= 1.0f)
        {

            playersprite->SetPosition(playerMoveTarget);
            playerMoving = false;

            if (pendingMoveSteps > 0)
            {
                return;
            }
            playerState = PlayerState::IDLE;
            UpdatePlayerAnimation();

            if (turnState == TurnState::PLAYER_MOVING)
            {
                if (pendingFastCard)
                {
                    std::cout << "[Fast Card] Move finished. Returning to PLAYER_TURN.\n";
                    turnState = TurnState::PLAYER_TURN;
                    pendingFastCard = false;
                }
                else
                {
                    std::cout << "[Card Move Finished] Enemy Turn Begins!\n";
                    turnState = TurnState::ENEMY_TURN;
                }
            }
            else
            {
                turnState = TurnState::PLAYER_TURN;
            }
        }


    }
}

void Level::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void Level::LevelFree()
{
    // 1. Clear card system (removes card layers from objectsList, nulls its own pointers)
    cardSystem.Clear(objectsList);

    // 2. Remove enemy-owned objects from objectsList before deleting enemy,
    //    to avoid double-free when the objectsList loop runs.
    if (enemy) {
        ImageObject* eObj = enemy->getObject();
        if (eObj) {
            auto it = std::find(objectsList.begin(), objectsList.end(), eObj);
            if (it != objectsList.end()) objectsList.erase(it);
            delete eObj;
            enemy->setObject(nullptr);
        }

        TextObject* hpTxt = enemy->getHPText();
        if (hpTxt) {
            auto it = std::find(objectsList.begin(), objectsList.end(), hpTxt);
            if (it != objectsList.end()) objectsList.erase(it);
            // Do NOT delete hpTxt here — Enemy destructor owns and frees it.
        }

        delete enemy;  // Enemy::~Enemy() deletes hpText
        enemy = nullptr;
    }

    // 3. Delete all remaining objects
    for (auto* obj : objectsList) {
        if (obj) delete obj;
    }
    objectsList.clear();

    // 4. Reset highlight state so LevelInit can recreate them
    attackHighlights.clear();
    highlightCreated = false;
    moveHighlights.clear();
    moveHighlightCreated = false;
    enemyAttackHighlights.clear();
    enemyHighlightCreated = false;

    // 5. Reset game state
    nowRow = 0;
    nowCol = 0;
    playerHealth = 5;
    playerMoving = false;
    playerAttacking = false;
    attackTimer = 0.0f;
    pendingAttack = false;
    pendingMoveSteps = 0;
    pendingMoveZone = -1;
    pendingFastCard = false;
    enemyPreparingAttack = false;
    tempDiscardDone = false;
    turnState = TurnState::PLAYER_TURN;
    player = nullptr;
    playersprite = nullptr;
    mainMenu = nullptr;
    hpBar = nullptr;
    hpMask = nullptr;

    cout << "Free Level" << endl;
}

void Level::LevelUnload()
{
    GameEngine::GetInstance()->ClearMesh();
    cout << "Unload Level" << endl;
}


// Input

void Level::HandleKey(char key)
{
    // Restart and quit are always allowed, regardless of turn state
    if (key == 'r') { GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART; return; }
    if (key == 'q') { GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;    return; }

    if (turnState != TurnState::PLAYER_TURN)
        return;

    switch (key)
    {
    case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL2;  break;
    default: break;
    }

    if (!playersprite) return;

	if (key == 'c') {
		currentPatternIndex++;
		if (currentPatternIndex >= patterns.size())
			currentPatternIndex = 0;

		//currentRotation = 0;
		rotatedPattern = patterns[currentPatternIndex];

		std::cout << "Switched to pattern #" << currentPatternIndex + 1 << std::endl;
	}
	if (key == 'x') {
		currentRotation = (currentRotation + 90) % 360;

		rotatedPattern = patterns[currentPatternIndex];
		int times = currentRotation / 90;
		for (int i = 0; i < times; ++i)
			rotatedPattern = rotatedPattern.rotated90CW();

		std::cout << "Rotated pattern to " << currentRotation << " degrees\n";
	}
	if (key == ' ') {
        auto attacks = rotatedPattern.applyTo(nowRow, nowCol);
        ApplyAttackCells(attacks);
	}
    if (key == 'p') {
        ApplyEnemyAttack();
    }
    if(key == 'l'){
        MoveEnemyTowardPlayer();
	}

    if (key == 'o') {
        enemy->rotatePattern();
        cout << "Enemy rotated pattern.\n";
    }

	//test player movement
    if (playerMoving) return;

    int targetRow = nowRow;
    int targetCol = nowCol;

    if (key == 'w' && nowCol > GridStartCol) {
        targetCol--;
        playerDir = PlayerDir::UP;
    }
    else if (key == 's' && nowCol < GridEndCol - 1) {
        targetCol++;
        playerDir = PlayerDir::DOWN;
    }
    else if (key == 'a' && nowRow > GridStartRow) {
        targetRow--;
        playerDir = PlayerDir::LEFT;
    }
    else if (key == 'd' && nowRow < GridEndRow - 1) {
        targetRow++;
        playerDir = PlayerDir::RIGHT;
    }
    else {
        return;
    }

    playerMoving = true;
    playerMoveTimer = 0.0f;

    playerMoveStart = playersprite->GetPosition();
    playerMoveTarget = GridToWorld(targetRow, targetCol);

    SetPlayerWalk(playerDir);

    nowRow = targetRow;
    nowCol = targetCol;
}

void Level::HandleMouse(int type, int x, int y)
{
    if (turnState != TurnState::PLAYER_TURN)
        return;

    int winW = GameEngine::GetInstance()->GetWindowWidth();
    int winH = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

    float realX = (x - winW / 2.0f) * (scaleW / winW);
    float realY = (winH / 2.0f - y) * (scaleH / winH);
    glm::vec3 mousePos(realX, realY, 0.0f);

    // ----------------------------------------------------
    // Hover
    // ----------------------------------------------------
    if (type == 3) {
        cardSystem.UpdateHover(mousePos, cardSystem.IsDragging(), objectsList);
        return;
    }

    // Left Click
    if (type == 0)
    {
        // menu toggle
        if (realX >= 875 && realX <= 925 && realY <= 530 && realY >= 470)
        {
            if (!Button::getMenu()) {
                Button::setMenu(true);
                if (mainMenu) {
                    mainMenu->SetPosition(glm::vec3(0, 0, 0));
                }
            }
            else {
                Button::setMenu(false);
                if (mainMenu) {
                    mainMenu->SetPosition(glm::vec3(0, 20000, 0));
                }
            }
        }

        // DRAW PILE BUTTON (RIGHT) - Left Click to discard hand and draw new cards
        if (cardSystem.IsDrawPileClicked(mousePos))
        {
            bool shouldEndTurn = cardSystem.UseDrawPileTurn();
            cardSystem.DiscardHandAndDraw(5, objectsList);
            if (shouldEndTurn)
            {
                tempDiscardDone = true;
                turnState = TurnState::ENEMY_TURN;
            }
            return;
        }

        // DISCARD PILE BUTTON (LEFT) - Left Click to view discard pile
        if (cardSystem.IsDiscardPileClicked(mousePos))
        {
            cardSystem.PrintDiscardPile();
            return;
        }

        // ------------------------------
        // DROP ZONES
        // ------------------------------
        cardSystem.UpdateHover(mousePos, false, objectsList);

        ImageObject* peek = cardSystem.PeekAt(mousePos);
        cardSystem.SetPendingCard(peek);

        /*if (testMove) {
            testMoveTarget = glm::vec3(realX, realY, 0);
            testMoveMoving = true;
        }*/
    }

    // ----------------------------------------------------
    // DRAG
    // ----------------------------------------------------
    if (type == 1)
    {
        ImageObject* pending = cardSystem.GetPendingCard();
        if (pending) {
            if (!cardSystem.IsDragging()) {
                cardSystem.BeginDrag(pending, mousePos, objectsList);
            }
            else {
                cardSystem.UpdateDrag(mousePos);

                int dz = cardSystem.HitDropZone(mousePos);
                if (dz >= 0)
                {
                    Card* cardData = cardSystem.FindCardByImage(cardSystem.GetDraggingCard());
                    PreviewAttackPattern(cardData, dz);

                    int moveSteps = 0;
                    int retreatSteps = 0;
                    for (Action* a : cardData->getActions())
                    {
                        if (auto* mv = dynamic_cast<MoveAction*>(a)) {
                            if (mv->isRetreat())
                                retreatSteps += mv->getValue();
                            else
                                moveSteps += mv->getValue();
                        }
                    }

                    int retreatDir = dz;
                    switch (dz)
                    {
                    case 0: retreatDir = 3; break;
                    case 1: retreatDir = 2; break;
                    case 2: retreatDir = 1; break;
                    case 3: retreatDir = 0; break;
                    }

                    if (retreatSteps > 0)
                        PreviewMovePath(retreatSteps, retreatDir);
                    else if (moveSteps > 0)
                        PreviewMovePath(moveSteps, dz);
                }
                else
                {
                    HideMoveHighlights();
                    HideAttackHighlights();
                }
            }
        }
        cardSystem.UpdateHover(mousePos, true, objectsList);
    }
    
    // ----------------------------------------------------
    // RELEASE (type == 2)
    // ----------------------------------------------------
    if (type == 2)
    {
        // Check if right-clicked on DRAW PILE BUTTON to view draw deck
        if (cardSystem.IsDrawPileClicked(mousePos))
        {
            cardSystem.PrintDrawDeck();
            return;
        }

        // Handle drag release
        if (cardSystem.IsDragging())
        {
            ImageObject* dragCard = cardSystem.GetDraggingCard();
            int dz = cardSystem.HitDropZone(mousePos);

            if (dz >= 0)
            {
                switch (dz)
                {
                case 0: playerDir = PlayerDir::LEFT;  break;
                case 1: playerDir = PlayerDir::UP;    break;
                case 2: playerDir = PlayerDir::DOWN;  break;
                case 3: playerDir = PlayerDir::RIGHT; break;
                }

                Card* cardData = cardSystem.FindCardByImage(dragCard);

                const char* zoneNames[4] = { "LEFT", "TOP", "BOTTOM", "RIGHT" };

                int moveSteps = 0;
                int retreatSteps = 0;

                struct PendingAttackInfo {
                    AttackAction* atk;
                    const AttackPattern* pattern;
                };
                std::vector<PendingAttackInfo> pendingAttacks;

                if (cardData)
                {
                    std::cout << "[Card] " << cardData->getName() << std::endl;
                    
                    // Apply overclock if this card has overclock value
                    if (cardData->getOverclockValue() > 0)
                    {
                        cardSystem.ApplyOverclock(cardData->getOverclockValue());
                    }

                    const auto& acts = cardData->getActions();
                    for (Action* a : acts)
                    {
                        if (auto* atk = dynamic_cast<AttackAction*>(a))
                        {
                            std::cout << "  AttackAction: " << atk->getValue() << std::endl;

                            const AttackPattern* basePat = cardSystem.GetDataLoader().getPatternForAction(a);
                            if (!basePat) {
                                std::cout << "    (no attack pattern linked to this action)\n";
                                pendingAttacks.push_back({ atk, nullptr });
                            }
                            else {
                                pendingAttacks.push_back({ atk, basePat });
                            }
                        }
                        else if (auto* mv = dynamic_cast<MoveAction*>(a))
                        {
                            if (mv->isRetreat())
                            {
                                retreatSteps += mv->getValue();
                                std::cout << "  RetreatAction: " << mv->getValue() << std::endl;
                            }
                            else
                            {
                                moveSteps += mv->getValue();
                                std::cout << "  MoveAction: " << mv->getValue() << std::endl;
                            }
                        }
                    }

                    // Determine move direction
                    // Retreat goes OPPOSITE of dz (attack forward, move backward)
                    int retreatDir = dz;
                    switch (dz)
                    {
                    case 0: retreatDir = 3; break; // LEFT -> RIGHT
                    case 1: retreatDir = 2; break; // UP -> DOWN
                    case 2: retreatDir = 1; break; // DOWN -> UP
                    case 3: retreatDir = 0; break; // RIGHT -> LEFT
                    }

                    // Combine: forward move uses dz, retreat uses opposite
                    // For cards with both (e.g. atk + re), retreat happens after attack
                    int totalMoveSteps = moveSteps + retreatSteps;
                    int moveDir = dz;
                    if (retreatSteps > 0 && moveSteps == 0)
                    {
                        moveDir = retreatDir;
                    }

                    bool hasAttack = !pendingAttacks.empty();
                    bool isFastCard = cardData->getIsFast();
                    pendingFastCard = isFastCard;

                    if (hasAttack)
                    {
                        playerState = PlayerState::ATTACK;
                        UpdatePlayerAnimation();

                        playerAttacking = true;
                        attackTimer = 0;

                        std::cout << "[Attack Animation Started]\n";

                        // After attack: retreat in opposite direction
                        if (retreatSteps > 0)
                        {
                            pendingMoveSteps = retreatSteps;
                            pendingMoveZone = retreatDir;
                        }
                        else
                        {
                            pendingMoveSteps = moveSteps;
                            pendingMoveZone = dz;
                        }

                        cardSystem.DecrementDrawPileTurn();
                        turnState = TurnState::PLAYER_MOVING;
                    }
                    else if (totalMoveSteps > 0 && playersprite)
                    {
                        pendingMoveSteps = totalMoveSteps;
                        pendingMoveZone = moveDir;

                        playerState = PlayerState::WALK;
                        UpdatePlayerAnimation();

                        cardSystem.DecrementDrawPileTurn();
                        turnState = TurnState::PLAYER_MOVING;

                        std::cout << "[Card Move] Player will walk "
                            << pendingMoveSteps << " steps toward " << zoneNames[moveDir] << "\n";
                    }
                    else
                    {
                        playerState = PlayerState::IDLE;
                        UpdatePlayerAnimation();

                        cardSystem.DecrementDrawPileTurn();

                        if (!isFastCard)
                        {
                            tempDiscardDone = false;
                            turnState = TurnState::ENEMY_TURN;
                        }
                    }

                    for (const PendingAttackInfo& pa : pendingAttacks)
                    {
                        AttackAction* atk = pa.atk;
                        const AttackPattern* basePat = pa.pattern;

                        if (!basePat) {
                            continue;
                        }

                        AttackPattern oriented = *basePat;
                        int rotateTimes = 0;

                        PlayerDir faceDir;

                        switch (dz)
                        {
                        case 0: faceDir = PlayerDir::LEFT;  break;
                        case 1: faceDir = PlayerDir::UP;    break;
                        case 2: faceDir = PlayerDir::DOWN;  break;
                        case 3: faceDir = PlayerDir::RIGHT; break;
                        }

                        switch (faceDir)
                        {
                        case PlayerDir::UP:    rotateTimes = 2; break;
                        case PlayerDir::RIGHT: rotateTimes = 1; break;
                        case PlayerDir::DOWN:  rotateTimes = 0; break;
                        case PlayerDir::LEFT:  rotateTimes = 3; break;
                        }
                        rotateTimes = (rotateTimes + 3) % 4;
                        for (int i = 0; i < rotateTimes; i++) {
                            oriented = oriented.rotated90CW();
                        }

                        auto cells = oriented.applyTo(nowRow, nowCol);
                        std::cout << "    Applying attack pattern from ("
                            << nowRow << ", " << nowCol << ")\n";

                        for (auto& cell : cells)
                        {
                            int gx = cell.first.x;
                            int gy = cell.first.y;

                            if (gx < GridStartRow || gx >= GridEndRow ||
                                gy < GridStartCol || gy >= GridEndCol)
                            {
                                std::cout << "      Skip out-of-bounds cell (" << gx << ", " << gy << ")\n";
                                continue;
                            }

                            std::cout << "      Attack cell (" << gx << ", " << gy << ")\n";

                            if (enemy &&
                                enemy->getNowRow() == gx &&
                                enemy->getNowCol() == gy)
                            {
                                enemy->getDamage(atk->getValue());
                                std::cout << "        HIT enemy! HP: " << enemy->getHealth() << std::endl;

                                if (enemy->getHealth() <= 0) {
                                    std::cout << "        Enemy died!\n";
                                }
                            }
                        }
                    }

                    // Reset overclock after an attack card is played
                    if (hasAttack)
                    {
                        cardSystem.ResetOverclock();
                    }

                    // Handle del (delete) vs normal discard
                    if (cardData->getIsDeleteAfterUse())
                    {
                        std::cout << "[Del] Card moved to delete pile.\n";
                        cardSystem.EndDragConfirmDelete(dragCard, objectsList);
                    }
                    else
                    {
                        cardSystem.EndDragConfirm(dragCard, objectsList);
                    }

                    if (isFastCard)
                    {
                        std::cout << "[Fast] Will not consume player turn.\n";
                    }
                }
                else
                {
                    std::cout << "[Warning] No Card* bound to this image" << std::endl;
                    cardSystem.EndDragConfirm(dragCard, objectsList);
                }

                std::cout << "----------------------------------------" << std::endl;
            }
            else
            {
                cardSystem.EndDragCancel(mousePos, objectsList);
            }
        }

        cardSystem.SetPendingCard(nullptr);
        cardSystem.UpdateHover(mousePos, false, objectsList);
        HideMoveHighlights();
        HideAttackHighlights();
    }

    /*if (player) {
        player->SetPosition(glm::vec3(realX, realY, 0));
    }*/
}


glm::vec3 Level::GridToWorld(int row, int col) const
{
    float x = row * 101.0f - 404.0f;
    float y = col * -105.0f + 352.0f;
    return glm::vec3(x, y, 0.0f);
}

void Level::AttackHighlights(std::vector<DrawableObject*>& list)
{
    if (highlightCreated) return;
    highlightCreated = true;

    attackHighlights.reserve(20);

    for (int i = 0; i < 20; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(GridWide, GridHigh);
        h->SetColor(1, 1, 1, 0.4f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        attackHighlights.push_back(h);
        list.push_back(h);
    }
}

void Level::HideAttackHighlights()
{
    for (auto* h : attackHighlights)
        h->SetPosition(glm::vec3(99999, 99999, 50));
}

void Level::MoveHighlights(std::vector<DrawableObject*>& list)
{
    if (moveHighlightCreated) return;
    moveHighlightCreated = true;

    moveHighlights.reserve(10);

    for (int i = 0; i < 10; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(GridWide/2, GridHigh/2);
        h->SetColor(0.2f, 0.5f, 1.0f, 0.4f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        moveHighlights.push_back(h);
        list.push_back(h);
    }
}

void Level::HideMoveHighlights()
{
    for (auto* h : moveHighlights)
        h->SetPosition(glm::vec3(99999, 99999, 50));
}

void Level::EnemyAttackHighlights(std::vector<DrawableObject*>& list)
{
    if (enemyHighlightCreated) return;
    enemyHighlightCreated = true;

    enemyAttackHighlights.reserve(20);

    for (int i = 0; i < 20; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(GridWide, GridHigh);
        h->SetColor(1.0f, 0.2f, 0.2f, 0.45f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        enemyAttackHighlights.push_back(h);
        list.push_back(h);
    }
}

void Level::HideEnemyAttackHighlights()
{
    for (auto* h : enemyAttackHighlights)
        h->SetPosition(glm::vec3(99999, 99999, 50));
}

// Apply attack pattern (player)
void Level::ApplyAttackCells(const std::vector<std::pair<IVec2, int>>& cells)
{
    cout << "Using pattern #" << currentPatternIndex + 1 << endl;

    for (const auto& item : cells)
    {
        const IVec2& cell = item.first;
        int x = cell.x;
        int y = cell.y;

        // Out-of-bound check
        if (x < 0 || x >= GridEndRow || y < 0 || y >= GridEndCol) {
            cout << "Skipped out-of-bound attack at (" << x << ", " << y << ")\n";
            continue;
        }

        cout << "attack at (" << x << ", " << y << ")\n";

        // Check if enemy is hit
        if (enemy && enemy->getNowRow() == x && enemy->getNowCol() == y) {
            enemy->getDamage(1);
            cout << "  HIT!!! Enemy HP: " << enemy->getHealth() << endl;

            if (enemy->getHealth() <= 0) {
                cout << "  Enemy died!" << endl;
            }
        }
    }

    cout << endl;
}

void Level::ApplyEnemyAttack()
{
    if (!enemy) return;
    enemy->showAttackText();
    auto attacks = enemy->getCurrentPattern().applyTo(enemy->getNowRow(), enemy->getNowCol());

    cout << "[Enemy Attack]\n";

    for (auto& cell : attacks)
    {
        int x = cell.first.x;
        int y = cell.first.y;

        if (x < 0 || x >= GridEndRow || y < 0 || y >= GridEndCol) {
            cout << "  Skip (" << x << ", " << y << ") out of bounds\n";
            continue;
        }

        cout << "  Enemy attacks (" << x << ", " << y << ")\n";

        if (nowRow == x && nowCol == y)
        {
            playerHealth--;
            cout << "    HIT PLAYER!!! New HP = " << playerHealth << endl;
            UpdateHPBar();
        }
    }

    cout << endl;
}

void Level::MoveEnemyTowardPlayer()
{
    if (!enemy) return;

    int er = enemy->getNowRow();
    int ec = enemy->getNowCol();

    int pr = nowRow;
    int pc = nowCol;

    int newR = er;
    int newC = ec;

    if (er < pr) newR = er + 1;
    else if (er > pr) newR = er - 1;
    else if (ec < pc) newC = ec + 1;
    else if (ec > pc) newC = ec - 1;

    newR = std::max(GridStartRow, std::min(newR, GridEndRow - 1));
    newC = std::max(GridStartCol, std::min(newC, GridEndCol - 1));

    enemy->setNowPosition(newR, newC);

    glm::vec3 world = GridToWorld(newR, newC);
    if (enemy->getObject())
        enemy->getObject()->SetPosition(world);

    std::cout << "Enemy moved to (" << newR << ", " << newC << ")\n";
}

bool Level::EnemyCanAttackPlayer()
{
    int er = enemy->getNowRow();
    int ec = enemy->getNowCol();
    int pr = nowRow;
    int pc = nowCol;

    return (abs(er - pr) <= 1 && abs(ec - pc) <= 1);
}

void Level::UpdateTurn()
{
    if (turnState == TurnState::PLAYER_MOVING)
        return;

    if (turnState == TurnState::PLAYER_TURN) {
        tempDiscardDone = false;
    }
    else if (turnState == TurnState::ENEMY_TURN)
    {
        if (!enemy)
        {
            turnState = TurnState::PLAYER_TURN;
            return;
        }

        if (!tempDiscardDone)
        {
            cardSystem.DiscardTempCardsFromHand(objectsList);
            tempDiscardDone = true;
        }

        if (enemyPreparingAttack)
        {
            cout << "[ENEMY TURN] Enemy attacks now!\n";

            HideEnemyAttackHighlights();
            ApplyEnemyAttack();

            enemyPreparingAttack = false;

            turnState = TurnState::PLAYER_TURN;
            return;
        }

        if (EnemyCanAttackPlayer())
        {
            cout << "[ENEMY TURN] Enemy prepares attack (SHOW WARNING)!\n";

            enemyPreparingAttack = true;

            PreviewEnemyAttack();

            turnState = TurnState::PLAYER_TURN;
            return;
        }

        cout << "[ENEMY TURN] Enemy moves toward player\n";

        HideAttackHighlights();
        MoveEnemyTowardPlayer();

        turnState = TurnState::PLAYER_TURN;
    }
}

void Level::LevelRestart() 
{
    cout << "=== RESTART START ===" << endl;

    LevelFree();
    LevelUnload();
    LevelLoad();
    LevelInit();

    cout << "=== RESTART COMPLETE ===" << endl;
}

void Level::UpdatePlayerAnimation()
{
    if (!playersprite) return;

    if (playerState == PlayerState::IDLE)
    {
        switch (playerDir)
        {
        case PlayerDir::DOWN:  playersprite->SetAnimationLoop(0, 0, 2, 800); break;
        case PlayerDir::LEFT:  playersprite->SetAnimationLoop(0, 2, 2, 800); break;
        case PlayerDir::UP:    playersprite->SetAnimationLoop(0, 4, 2, 800); break;
        case PlayerDir::RIGHT: playersprite->SetAnimationLoop(0, 6, 2, 800); break;
        }
    }
    else if (playerState == PlayerState::WALK)
    {
        switch (playerDir)
        {
        case PlayerDir::UP:    playersprite->SetAnimationLoop(1, 8, 4, 150); break;
        case PlayerDir::DOWN:  playersprite->SetAnimationLoop(1, 0, 4, 150); break;
        case PlayerDir::RIGHT: playersprite->SetAnimationLoop(1, 12, 4, 150); break;
        case PlayerDir::LEFT:  playersprite->SetAnimationLoop(1, 4, 4, 150); break;
        }
    }
    else if (playerState == PlayerState::ATTACK)
    {
        switch (playerDir)
        {
        case PlayerDir::DOWN:  playersprite->SetAnimationLoop(2, 0, 8, 100); break;
        case PlayerDir::LEFT:  playersprite->SetAnimationLoop(2, 8, 8, 100); break;
        case PlayerDir::UP:    playersprite->SetAnimationLoop(3, 0, 8, 100); break;
        case PlayerDir::RIGHT: playersprite->SetAnimationLoop(3, 8, 8, 100); break;
        }
    }
}
void Level::SetPlayerIdle(PlayerDir dir)
{
    switch (dir)
    {
    case PlayerDir::DOWN:  playersprite->SetAnimationLoop(0, 0, 2, 800); break;
    case PlayerDir::LEFT:  playersprite->SetAnimationLoop(0, 2, 2, 800); break;
    case PlayerDir::UP:    playersprite->SetAnimationLoop(0, 4, 2, 800); break;
    case PlayerDir::RIGHT: playersprite->SetAnimationLoop(0, 6, 2, 800); break;
    }
}

void Level::SetPlayerWalk(PlayerDir dir)
{
    switch (dir)
    {
    case PlayerDir::DOWN:  playersprite->SetAnimationLoop(1, 0, 2, 150); break;
    case PlayerDir::UP:    playersprite->SetAnimationLoop(1, 4, 2, 150); break;
    case PlayerDir::RIGHT: playersprite->SetAnimationLoop(1, 6, 2, 150); break;
    case PlayerDir::LEFT:  playersprite->SetAnimationLoop(1, 2, 2, 150); break;
    }
}

void Level::PreviewAttackPattern(Card* cardData, int dz)
{
    if (!cardData) return;

    AttackHighlights(objectsList);
    HideAttackHighlights();

    AttackAction* atk = nullptr;
    const AttackPattern* pat = nullptr;

    for (Action* a : cardData->getActions())
    {
        atk = dynamic_cast<AttackAction*>(a);
        if (atk)
        {
            pat = cardSystem.GetDataLoader().getPatternForAction(a);
            break;
        }
    }

    if (!atk || !pat) return;

    AttackPattern oriented = *pat;
    int rotateTimes = 0;

    switch (dz)
    {
        case 0: rotateTimes = 2; break; // LEFT
        case 1: rotateTimes = 1; break; // UP
        case 2: rotateTimes = 3; break; // DOWN
        case 3: rotateTimes = 0; break; // RIGHT
    }

    for (int i = 0; i < rotateTimes; i++)
        oriented = oriented.rotated90CW();

    auto cells = oriented.applyTo(nowRow, nowCol);

    int index = 0;
    for (auto& cell : cells)
    {
        int gx = cell.first.x;
        int gy = cell.first.y;

        if (gx < GridStartRow || gx >= GridEndRow ||
            gy < GridStartCol || gy >= GridEndCol)
            continue;

        if (index >= attackHighlights.size()) break;

        glm::vec3 world = GridToWorld(gx, gy);
        attackHighlights[index]->SetPosition(glm::vec3(world.x, world.y, 30));
        index++;
    }
}

void Level::PreviewMovePath(int steps, int dir)
{
    MoveHighlights(objectsList);
    HideMoveHighlights();

    int r = nowRow;
    int c = nowCol;

    int lastValidR = r;
    int lastValidC = c;

    for (int i = 0; i < steps; i++)
    {
        switch (dir)
        {
        case 0: r--; break; // LEFT
        case 1: c--; break; // UP
        case 2: c++; break; // DOWN
        case 3: r++; break; // RIGHT
        }

        if (r < GridStartRow || r >= GridEndRow ||
            c < GridStartCol || c >= GridEndCol)
        {
            break;
        }

        if (enemy && enemy->getNowRow() == r && enemy->getNowCol() == c)
        {
            break;
        }

        lastValidR = r;
        lastValidC = c;
    }

    glm::vec3 world = GridToWorld(lastValidR, lastValidC);

    if (!moveHighlights.empty())
    {
        moveHighlights[0]->SetPosition(glm::vec3(world.x, world.y, 60));
    }
}


/*//Highlight the entire path (old version)
void Level::PreviewMovePath(int steps, int dir)
{
    MoveHighlights(objectsList);

    int r = nowRow;
    int c = nowCol;

    // simulate movement until final tile
    for (int i = 0; i < steps; i++)
    {
        switch (dir)
        {
        case 0: r--; break; // LEFT
        case 1: c--; break; // UP
        case 2: c++; break; // DOWN
        case 3: r++; break; // RIGHT
        }

        // stop if out of bounds
        if (r < GridStartRow || r >= GridEndRow ||
            c < GridStartCol || c >= GridEndCol)
            return;

        // stop if enemy blocks
        if (enemy && enemy->getNowRow() == r && enemy->getNowCol() == c)
            return;
    }

    HideMoveHighlights();

    glm::vec3 world = GridToWorld(r, c);

    if (!moveHighlights.empty())
        moveHighlights[0]->SetPosition(glm::vec3(world.x, world.y, 40));
}
*/
void Level::PreviewEnemyAttack()
{
    if (!enemy) return;

    EnemyAttackHighlights(objectsList);
    HideEnemyAttackHighlights();

    auto cells =
        enemy->getCurrentPattern().applyTo(
            enemy->getNowRow(),
            enemy->getNowCol()
        );

    int index = 0;

    for (auto& cell : cells)
    {
        int gx = cell.first.x;
        int gy = cell.first.y;

        if (gx < GridStartRow || gx >= GridEndRow ||
            gy < GridStartCol || gy >= GridEndCol)
            continue;

        if (index >= enemyAttackHighlights.size())
            break;

        glm::vec3 world = GridToWorld(gx, gy);

        enemyAttackHighlights[index]->SetPosition(
            glm::vec3(world.x, world.y, 40)
        );

        index++;
    }
}
void Level::UpdateHPBar()
{
    if (!hpBar || !hpMask) return;

    float fullWidth = 300.0f;

    playerHealth = std::max(0, std::min(playerHealth, maxPlayerHealth));

    float percent = (float)playerHealth / (float)maxPlayerHealth;

    hpBar->SetSize(fullWidth, -80.0f);

    float missingWidth = fullWidth * (1.0f - percent);


    hpMask->SetSize(missingWidth, -80.0f);
    float barLeftX = -800.0f;
    float maskX = barLeftX + (fullWidth - missingWidth) / 2.0f;

    hpMask->SetPosition(glm::vec3(maskX, 500.0f, 5.0f));
}

