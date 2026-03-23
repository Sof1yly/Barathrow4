#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "TriangleMeshVbo.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "BuffAction.h"
#include "DebuffAction.h"
#include "EnergyAction.h"
#include "CombineObject.h"
#include "TextObject.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_set>
#include <unordered_map>

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
	srand(time(NULL));
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

    highlightManager.Init(objectsList, GridWide, GridHigh);

    //Enemy
    Enemy* e1 = new Enemy(Enemy::EnemyType::A1);
	int ran1 = rand() % 8+1;
    e1->setNowPosition(ran1, 0);
    e1->SetWorldPosition(GridToWorld(ran1, 0));
	int ran2 = rand() % 8 + 1;
    Enemy* e2 = new Enemy(Enemy::EnemyType::A2);
    e2->setNowPosition(ran2, 2);
    e2->SetWorldPosition(GridToWorld(ran2, 2));
	Enemy* e3 = new Enemy(Enemy::EnemyType::A1);
	int ran3 = rand() % 8 + 1;
	e3->setNowPosition(ran3, 4);
	e3->SetWorldPosition(GridToWorld(ran3, 4));



    enemies.push_back(e1);
    enemies.push_back(e2);
	enemies.push_back(e3);

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        objectsList.push_back(e->getObject());
        objectsList.push_back(e->getHPText());
        objectsList.push_back(e->getCorruptText());
    }


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

    playerData.InitShieldUI(objectsList);

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
    if (!cardSystem.LoadData("../Resource/GameData/Pattern.txt", "../Resource/GameData/CardAction.txt", "../Resource/GameData/CardDesc.txt", &error)) {
        std::cerr << "Error loading card data: " << error << std::endl;
    }

    cardSystem.ShuffleDeck();

    // Start the game with 5 cards in hand
    cardSystem.DealNewHand(5, objectsList);

    // Card system UI (discard/draw pile buttons + drop zones)
    cardSystem.InitUI(objectsList);


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
    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        e->UpdateTextPosition();
        e->Update(deltaTime / 1000.0f);
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
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            if (e &&
                e->getNowRow() == targetRow &&
                e->getNowCol() == targetCol)
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
        highlightManager.HideAllPlayer();
        playerMoveTimer = 0.0f;

        playerMoveStart = playersprite->GetPosition();
        playerMoveTarget = GridToWorld(targetRow, targetCol);

        nowRow = targetRow;
        nowCol = targetCol;

        pendingMoveSteps--;
    }

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
    for (auto it = enemies.begin(); it != enemies.end(); )
    {
        Enemy* e = *it;

        if (e && e->getIsDead())
        {
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getObject()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getHPText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCorruptText()), objectsList.end());

            delete e;
            it = enemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
}

void Level::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void Level::LevelFree()
{
    HideCardInspect();

    // 1. Clear card system (removes card layers from objectsList, nulls its own pointers)
    cardSystem.Clear(objectsList);

    // 2. Remove enemy-owned objects from objectsList before deleting enemy,
    //    to avoid double-free when the objectsList loop runs.
    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        SpriteObject* obj = e->getObject();
        if (obj)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end()) objectsList.erase(it);
            delete obj;
        }

        delete e;
    }
    enemies.clear();

    // 3. Delete all remaining objects
    for (auto* obj : objectsList) {
        if (obj) delete obj;
    }
    objectsList.clear();

    // 4. Reset highlight state so LevelInit can recreate them
    highlightManager.Reset();

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
    lagTurns = 0;
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

    // Toggle deck viewer with 'v' key
    if (key == 'v')
    {
        HideCardInspect();

        if (deckViewer.IsActive())
        {
            deckViewer.Hide(objectsList);
        }
        else
        {
            // Show fixed snapshot of all cards (never changes when cards are played)
            const vector<Card*>& allCards = cardSystem.GetFullCollection();
            deckViewer.SetDeck(allCards);
            deckViewer.Show(objectsList);
        }
        return;
    }

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
		/*currentRotation = (currentRotation + 90) % 360;

		rotatedPattern = patterns[currentPatternIndex];
		int times = currentRotation / 90;
		for (int i = 0; i < times; ++i)
			rotatedPattern = rotatedPattern.rotated90CW();

		std::cout << "Rotated pattern to " << currentRotation << " degrees\n";
        */
        playerData.AddShield(10);
	}
	if (key == ' ') {

	}
    if (key == 'p') {
    }
    if(key == 'l'){
	}

    if (key == 'o') {
        
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
    // Calculate mouse position (needed for all mouse operations)
    int winW = GameEngine::GetInstance()->GetWindowWidth();
    int winH = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

    float realX = (x - winW / 2.0f) * (scaleW / winW);
    float realY = (winH / 2.0f - y) * (scaleH / winH);
    glm::vec3 mousePos(realX, realY, 0.0f);

    // ----------------------------------------------------
    // Right-Click Down (type 4)
    // ----------------------------------------------------
    if (type == 4)
    {
        if (cardSystem.IsDrawPileClicked(mousePos))
        {
            HideCardInspect();
            if (deckViewer.IsActive()) {
                deckViewer.Hide(objectsList);
            }

            const vector<Card*>& drawCards = cardSystem.GetDeck();
            deckViewer.SetDeck(drawCards);
            deckViewer.Show(objectsList);
            return;
        }

        if (cardSystem.IsDiscardPileClicked(mousePos))
        {
            HideCardInspect();
            if (deckViewer.IsActive()) {
                deckViewer.Hide(objectsList);
            }

            const vector<Card*>& discardCards = cardSystem.GetDiscard();
            deckViewer.SetDeck(discardCards);
            deckViewer.Show(objectsList);
            return;
        }

        if (deckViewer.IsActive())
        {
            deckViewer.NextPage(objectsList);
            return;
        }

        ImageObject* hit = cardSystem.PeekAt(mousePos);
        Card* cardData = hit ? cardSystem.FindCardByImage(hit) : nullptr;

        if (cardData)
        {
            if (cardInspectVisible && inspectedCard == cardData) {
                HideCardInspect();
            }
            else {
                ShowCardInspect(cardData);
            }
        }
        else
        {
            HideCardInspect();
        }
        return;
    }

    // ----------------------------------------------------
    // Right-Click Release (type 5) - Currently unused
    // ----------------------------------------------------
    if (type == 5)
    {
        return;
    }

    // If deck viewer is active, handle navigation
    if (deckViewer.IsActive())
    {
        // Left click goes to previous page
        if (type == 0)
        {
            deckViewer.PrevPage(objectsList);
        }
        return;
    }

    if (turnState != TurnState::PLAYER_TURN)
        return;

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
                    highlightManager.HideAllPlayer();
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
                int pendingDelayTurns = 0;
                int pendingCorruptionStacks = 0;

                if (cardData)
                {
                    // Check energy (con) requirement before allowing play
                    if (!cardSystem.CanPlayCard(cardData))
                    {
                        if (cardData->isEnergyCard())
                        {
                            std::cout << "[Blocked] " << cardData->getName()
                                      << " is an energy card and cannot be played." << std::endl;
                        }
                        else
                        {
                            int conReq = cardData->getConsumeRequirement();
                            int enrCount = cardSystem.CountEnergyCardsInHand();
                            std::cout << "[Blocked] " << cardData->getName()
                                      << " requires " << conReq << " energy card(s), but only "
                                      << enrCount << " in hand." << std::endl;
                        }
                        cardSystem.EndDragCancel(mousePos, objectsList);
                        cardSystem.SetPendingCard(nullptr);
                        cardSystem.UpdateHover(mousePos, false, objectsList);
                        highlightManager.HideAllPlayer();
                        return;
                    }

                    std::cout << "[Card] " << cardData->getName() << std::endl;

                    // Consume energy cards if this card has con requirement
                    int conReq = cardData->getConsumeRequirement();
                    if (conReq > 0)
                    {
                        cardSystem.ConsumeEnergyCards(conReq, objectsList);
                    }

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
                        else if (auto* buff = dynamic_cast<BuffAction*>(a))
                        {
                            if (buff->getSubType() == BuffSubType::Shield)
                            {
                                playerData.AddShield(buff->getValue());
                            }
                        }
                        else if (auto* debuff = dynamic_cast<DebuffAction*>(a))
                        {
                            if (debuff->getSubType() == DebuffSubType::Delay)
                            {
                                pendingDelayTurns += debuff->getValue();
                                std::cout << "  DelayAction: " << debuff->getValue() << std::endl;
                            }
                            else if (debuff->getSubType() == DebuffSubType::Corrupt)
                            {
                                pendingCorruptionStacks += debuff->getValue();
                                std::cout << "  CorruptAction: " << debuff->getValue() << std::endl;
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

                    bool corruptionApplied = false;
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
                            for (auto* e : enemies)
                            {
                                if (!e || e->getIsDead()) continue;
                                if (e &&
                                    e->getNowRow() == gx &&
                                    e->getNowCol() == gy)
                                {
                                    e->getDamage(atk->getValue());
                                    std::cout << "        HIT enemy! HP: " << e->getHealth() << std::endl;

                                    if (pendingDelayTurns > 0)
                                    {
                                        e->addDelay(pendingDelayTurns);
                                    }

                                    if (!corruptionApplied && pendingCorruptionStacks > 0)
                                    {
                                        e->addCorruption(pendingCorruptionStacks);
                                        corruptionApplied = true;
                                    }

                                    if (e->getHealth() <= 0) {
                                        std::cout << "        Enemy died!\n";
                                    }
                                }
                            }
                        }
                    }

                    // Reset overclock after an attack card is played
                    if (hasAttack)
                    {
                        cardSystem.ResetOverclock();
                    }

                    // Generate energy cards if this card has gen action
                    int genCount = cardData->getGenerateCount();
                    if (genCount > 0)
                    {
                        std::cout << "[Energy] Card generates " << genCount << " energy card(s)." << std::endl;
                        cardSystem.GenerateEnergyCards(genCount, objectsList);
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

                    if (cardData->getIsLag())
                    {
                        lagTurns++;
                        std::cout << "[Lag] Player will skip next turn. Lag turns: " << lagTurns << std::endl;
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
        highlightManager.HideAllPlayer();
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
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            if (e->getNowRow() == x && e->getNowCol() == y)
            {
                e->getDamage(1);
                cout << "  HIT!!! Enemy HP: " << e->getHealth() << endl;
            }
        }
    }

    cout << endl;
}

void Level::ApplyEnemyAttack(Enemy* e)
{
    if (!e) return;

    e->PlayAttackAnimation(playersprite->GetPosition());
    e->showAttackText();

    auto attacks = e->getCurrentPattern().applyTo(
        e->getNowRow(), e->getNowCol());

    for (auto& cell : attacks)
    {
        int x = cell.first.x;
        int y = cell.first.y;

        if (nowRow == x && nowCol == y)
        {
            int damage = 1;
            damage = playerData.AbsorbDamage(damage);

            if (damage > 0)
            {
                playerHealth -= damage;
                UpdateHPBar();
            }
        }
    }
}

bool Level::EnemyCanAttackPlayer(Enemy* e)
{
    if (!e) return false;

    int er = e->getNowRow();
    int ec = e->getNowCol();

    return (abs(er - nowRow) <= 1 && abs(ec - nowCol) <= 1);
}

void Level::UpdateTurn()
{
    if (turnState == TurnState::PLAYER_MOVING)
        return;

    switch (turnState)
    {
    case TurnState::PLAYER_TURN:
        // Do nothing.
        // Player input drives this state.
        break;

    case TurnState::ENEMY_TURN:
    {

        if (enemies.empty())
        {
            turnState = TurnState::PLAYER_TURN;
            return;
        }

        if (!tempDiscardDone)
        {
            cardSystem.DiscardTempCardsFromHand(objectsList);
            tempDiscardDone = true;
        }
        highlightManager.HideAllEnemy();

        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            if (e->isDelayed())
            {
                e->decrementDelay();
                continue;
            }

            if (e->isPreparingAttack())
            {
                ApplyEnemyAttack(e);
                e->setPreparingAttack(false);
            }
            else if (EnemyCanAttackPlayer(e))
            {
                e->setPreparingAttack(true);
                PreviewEnemyAttack(e);
            }
            else
            {
                e->MoveTowardPlayer(
                    nowRow, nowCol,
                    GridStartRow, GridEndRow,
                    GridStartCol, GridEndCol,
                    enemies
                );
                glm::vec3 world = GridToWorld(e->getNowRow(), e->getNowCol());
                e->SetWorldPosition(world);
            }
        }
        turnState = TurnState::END_TURN;
        break;

    }

    case TurnState::END_TURN:
    {
        EndTurn();
        break;
    }
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

void Level::HideCardInspect()
{
    if (!cardInspectVisible && cardInspectObjects.empty()) return;

    for (DrawableObject* obj : cardInspectObjects)
    {
        if (!obj) continue;

        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }

        delete obj;
    }

    cardInspectObjects.clear();
    inspectedCard = nullptr;
    cardInspectVisible = false;
}

std::string Level::GetKeywordTitle(const std::string& code) const
{
    if (code == "atk") return "Attack";
    if (code == "mov") return "Move";
    if (code == "re")  return "Retreat";
    if (code == "oc")  return "Overclock";
    if (code == "shi") return "Shield";
    if (code == "de")  return "Delay";
    if (code == "cor") return "Corrupt";
    if (code == "gen") return "Generate";
    if (code == "con") return "Consume";
    if (code == "enr") return "Energy";
    return code;
}

std::string Level::BuildCardInspectText(Card* cardData) const
{
    if (!cardData) return "";

    const GameDataLoader& loader = cardSystem.GetDataLoader();
    std::unordered_set<std::string> seen;
    std::vector<std::string> orderedCodes;
    std::unordered_map<std::string, int> values;

    for (Action* a : cardData->getActions())
    {
        if (!a) continue;

        std::string code = a->getActionCode();
        if (code.empty()) continue;

        if (seen.insert(code).second) {
            orderedCodes.push_back(code);
        }

        values[code] = a->getValue();
    }

    if (cardData->getOverclockValue() > 0)
    {
        std::string ocCode = "oc";
        if (seen.insert(ocCode).second) {
            orderedCodes.push_back(ocCode);
        }
        values[ocCode] = cardData->getOverclockValue();
    }

    std::string out;
    bool first = true;

    for (const std::string& code : orderedCodes)
    {
        std::string desc = loader.getActionDescription(code);
        if (desc.empty()) continue;

        auto it = values.find(code);
        if (it != values.end())
        {
            const std::string valueText = std::to_string(it->second);
            std::size_t pos = 0;
            while ((pos = desc.find("X", pos)) != std::string::npos)
            {
                desc.replace(pos, 1, valueText);
                pos += valueText.size();
            }
        }

        if (!first) out += "\n\n";
        out += GetKeywordTitle(code) + "\n" + desc;
        first = false;
    }

    return out;
}

void Level::BuildCardInspectGrid(Card* cardData)
{
    std::vector<std::pair<IVec2, int>> cells;

    if (cardData)
    {
        for (Action* a : cardData->getActions())
        {
            AttackAction* atk = dynamic_cast<AttackAction*>(a);
            if (!atk) continue;

            const AttackPattern* pat = cardSystem.GetDataLoader().getPatternForAction(a);
            if (pat) {
                cells = pat->applyTo(0, 0);
                break;
            }
        }
    }

    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (const auto& c : cells)
    {
        minX = std::min(minX, c.first.x);
        maxX = std::max(maxX, c.first.x);
        minY = std::min(minY, c.first.y);
        maxY = std::max(maxY, c.first.y);
    }

    const float cellSize = 78.0f;
    const float centerX = 560.0f;
    const float centerY = 30.0f;
    const float z = 915.0f;

    const float totalW = (maxX - minX + 1) * cellSize;
    const float totalH = (maxY - minY + 1) * cellSize;
    const float left = centerX - totalW * 0.5f + cellSize * 0.5f;
    const float top = centerY + totalH * 0.5f - cellSize * 0.5f;

    std::unordered_set<long long> attackSet;
    for (const auto& c : cells)
    {
        long long key = (static_cast<long long>(c.first.x) << 32) ^ static_cast<unsigned int>(c.first.y);
        attackSet.insert(key);
    }

    for (int gx = minX; gx <= maxX; ++gx)
    {
        for (int gy = minY; gy <= maxY; ++gy)
        {
            GameObject* tile = new GameObject();
            tile->SetSize(cellSize - 10.0f, -(cellSize - 10.0f));

            float px = left + (gx - minX) * cellSize;
            float py = top - (gy - minY) * cellSize;
            tile->SetPosition(glm::vec3(px, py, z));

            const bool isOrigin = (gx == 0 && gy == 0);
            long long key = (static_cast<long long>(gx) << 32) ^ static_cast<unsigned int>(gy);
            const bool isAttack = attackSet.find(key) != attackSet.end();

            if (isOrigin) {
                tile->SetColor(0.95f, 0.95f, 0.95f, 0.95f);
            }
            else if (isAttack) {
                tile->SetColor(0.95f, 0.25f, 0.15f, 0.95f);
            }
            else {
                tile->SetColor(0.80f, 0.80f, 0.80f, 0.90f);
            }

            cardInspectObjects.push_back(tile);
            objectsList.push_back(tile);
        }
    }
}

void Level::ShowCardInspect(Card* cardData)
{
    if (!cardData) {
        HideCardInspect();
        return;
    }

    HideCardInspect();

    GameObject* panel = new GameObject();
    panel->SetSize(1750.0f, -840.0f);
    panel->SetPosition(glm::vec3(0.0f, 20.0f, 900.0f));
    panel->SetColor(0.33f, 0.33f, 0.33f, 0.88f);
    cardInspectObjects.push_back(panel);
    objectsList.push_back(panel);

    if (!cardData->HasVisuals()) {
        cardData->CreateVisuals();
    }

    auto cloneCardLayer = [&](ImageObject* src) {
        if (!src) return;
        ImageObject* copy = new ImageObject();
        copy->SetTextureId(src->GetTextureId());
        copy->SetSize(364.0f, -533.0f);
        copy->SetPosition(glm::vec3(-640.0f, 40.0f, 910.0f));
        cardInspectObjects.push_back(copy);
        objectsList.push_back(copy);
    };

    cloneCardLayer(cardData->GetBackground());
    cloneCardLayer(cardData->GetStarOverlay());
    cloneCardLayer(cardData->GetTypeIcon());
    cloneCardLayer(cardData->GetVisual());
    cloneCardLayer(cardData->GetCardFrame());

    const float inspectCardW = 364.0f;
    const float inspectCardH = 533.0f;
    const glm::vec3 inspectCardPos(-640.0f, 40.0f, 910.0f);

    if (cardData->GetNameText())
    {
        TextObject* nameOnCard = new TextObject();
        SDL_Color cardNameColor = { 245, 245, 245, 255 };
        nameOnCard->LoadText(cardData->getName(), cardNameColor, 22);

        glm::vec3 local = cardData->GetNameText()->GetLocalPosition();
        float leftAnchorX = (inspectCardPos.x - (inspectCardW * 0.5f)) + (local.x * inspectCardW);
        float centeredX = leftAnchorX + (nameOnCard->GetSize().x * 0.5f);
        float centeredY = inspectCardPos.y + (local.y * inspectCardH);
        nameOnCard->SetPosition(glm::vec3(centeredX, centeredY, 912.0f));

        cardInspectObjects.push_back(nameOnCard);
        objectsList.push_back(nameOnCard);
    }

    if (cardData->GetDescriptionText())
    {
        std::string cardDesc = cardData->getDescription();
        for (Action* a : cardData->getActions())
        {
            if (!a) continue;
            const std::string token = "{" + a->getActionCode() + "}";
            const std::string value = std::to_string(a->getValue());
            std::size_t pos = 0;
            while ((pos = cardDesc.find(token, pos)) != std::string::npos)
            {
                cardDesc.replace(pos, token.size(), value);
                pos += value.size();
            }
        }

        if (cardData->getOverclockValue() > 0)
        {
            const std::string token = "{oc}";
            const std::string value = std::to_string(cardData->getOverclockValue());
            std::size_t pos = 0;
            while ((pos = cardDesc.find(token, pos)) != std::string::npos)
            {
                cardDesc.replace(pos, token.size(), value);
                pos += value.size();
            }
        }

        TextObject* descOnCard = new TextObject();
        SDL_Color cardDescColor = { 220, 220, 220, 255 };
        descOnCard->LoadTextWrapped(cardDesc, cardDescColor, 18, 300);

        glm::vec3 local = cardData->GetDescriptionText()->GetLocalPosition();
        float leftAnchorX = (inspectCardPos.x - (inspectCardW * 0.5f)) + (local.x * inspectCardW);
        float centeredX = leftAnchorX + (descOnCard->GetSize().x * 0.5f);
        float centeredY = inspectCardPos.y + (local.y * inspectCardH);
        descOnCard->SetPosition(glm::vec3(centeredX, centeredY, 912.0f));

        cardInspectObjects.push_back(descOnCard);
        objectsList.push_back(descOnCard);
    }

    TextObject* title = new TextObject();
    title->SetPosition(glm::vec3(-120.0f, 250.0f, 920.0f));
    SDL_Color titleColor = { 245, 245, 245, 255 };
    title->LoadText(cardData->getName(), titleColor, 46);
    cardInspectObjects.push_back(title);
    objectsList.push_back(title);

    std::string keywordText = BuildCardInspectText(cardData);
    if (!keywordText.empty())
    {
        TextObject* body = new TextObject(); 
        body->SetPosition(glm::vec3(-80.0f, 105.0f, 920.0f));
        SDL_Color bodyColor = { 230, 230, 230, 255 };
        body->LoadTextWrapped(keywordText, bodyColor, 28, 700);
        cardInspectObjects.push_back(body);
        objectsList.push_back(body);
    }

    BuildCardInspectGrid(cardData);

    inspectedCard = cardData;
    cardInspectVisible = true;
}

void Level::PreviewAttackPattern(Card* cardData, int dz)
{
    if (!cardData) return;

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
    case 0: rotateTimes = 2; break;
    case 1: rotateTimes = 1; break;
    case 2: rotateTimes = 3; break;
    case 3: rotateTimes = 0; break;
    }

    for (int i = 0; i < rotateTimes; i++)
        oriented = oriented.rotated90CW();

    auto cells = oriented.applyTo(nowRow, nowCol);

    highlightManager.ShowAttackPattern(
        cells,
        GridStartRow, GridEndRow,
        GridStartCol, GridEndCol,
        [this](int r, int c) { return GridToWorld(r, c); }
    );
}

void Level::PreviewMovePath(int steps, int dir)
{
    std::vector<std::pair<int, int>> enemyPositions;

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        if (e)
        {
            enemyPositions.emplace_back(
                e->getNowRow(),
                e->getNowCol()
            );
        }
    }

    highlightManager.ShowMovePreview(
        nowRow,
        nowCol,
        steps,
        dir,
        GridStartRow, GridEndRow,
        GridStartCol, GridEndCol,
        [this](int r, int c) { return GridToWorld(r, c); },
        enemyPositions
    );
}

void Level::PreviewEnemyAttack(Enemy* e)
{
    if (!e) return;

    auto cells = e->getCurrentPattern().applyTo(
        e->getNowRow(),
        e->getNowCol()
    );

    highlightManager.ShowEnemyAttack(
        cells,
        GridStartRow, GridEndRow,
        GridStartCol, GridEndCol,
        [this](int r, int c) { return GridToWorld(r, c); }
    );
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

void Level::EndTurn()
{
    turnCount++;
    playerData.ResetShield();
	cout << "\n\n\n\n\n";
	cout << "=== END OF TURN , Now Turn  =  " << turnCount << " ===\n\n";

    if (lagTurns > 0)
    {
        lagTurns--;
        std::cout << "[Lag] Skipping player turn! Remaining lag: " << lagTurns << std::endl;
        tempDiscardDone = false;
        turnState = TurnState::ENEMY_TURN;
    }
    else
    {
        turnState = TurnState::PLAYER_TURN;
    }
}
