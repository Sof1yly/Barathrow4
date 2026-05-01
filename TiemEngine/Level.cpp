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
//
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
            tile->SetPosition(glm::vec3(i * 101.0f - 404.0f,j * -105.0f + 352.0f, 0.0f));
            objectsList.push_back(tile);
        }
    }

    highlightManager.Init(objectsList, GridWide, GridHigh);

    //Load Enemy
    EnemyDatabase::LoadFromFile("../Resource/GameData/EnemyData.txt");//data
    EnemyLoadPattern::LoadFromFile("../Resource/GameData/EnemyPattern.txt");//pattern

    //Enemy
    Enemy* e1 = new Enemy((rand() % 2 == 0) ? Enemy::EnemyType::A : Enemy::EnemyType::B);
    int ran1 = rand() % 8 + 1;
    e1->setNowPosition(ran1, 0);
    e1->SetWorldPosition(GridToWorld(ran1, 0));

    Enemy* e2 = new Enemy((rand() % 2 == 0) ? Enemy::EnemyType::A : Enemy::EnemyType::B);
    int ran2 = rand() % 8 + 1;
    e2->setNowPosition(ran2, 2);
    e2->SetWorldPosition(GridToWorld(ran2, 2));

    Enemy* e3 = new Enemy((rand() % 2 == 0) ? Enemy::EnemyType::A : Enemy::EnemyType::B);
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
        objectsList.push_back(e->getDebuffText());
    }


    // 3) Player sprite (3x4, 192x256)
    {
        glm::vec3 startPos = GridToWorld(nowRow, nowCol);
        playerData.InitSprite(objectsList, startPos);
        playersprite = playerData.GetSprite();
    }

    {
        /*GameObject* obj2 = new GameObject();
        obj2->SetColor(0.0f, 1.0f, 0.0f);
        obj2->SetSize(50.0f, 50.0f);
        obj2->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
		objectsList.push_back(obj2);*/ //Green obj dont delete, for testing and save UI position
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
        viewDeckButton.InitPreset(Button::Preset::ViewDeck, objectsList);
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
    if (!cardSystem.LoadData("../Resource/GameData/Pattern.txt", "../Resource/GameData/CardActionStandard.txt", "../Resource/GameData/CardDesc.txt", &error)) {
        std::cerr << "Error loading card data: " << error << std::endl; //CardActionStandard.txt is for starter card , CardDesc_filled for all the card in the game.
    }

    if (!cardRewardSystem.LoadPoolData("../Resource/GameData/Pattern.txt", "../Resource/GameData/CardDesc_filled.txt", "../Resource/GameData/CardDesc.txt", &error)) {
        std::cerr << "Error loading reward card pool: " << error << std::endl;
    }

    if (!shopSystem.LoadPoolData("../Resource/GameData/Pattern.txt", "../Resource/GameData/CardDesc_filled.txt", "../Resource/GameData/CardDesc.txt", &error)) {
        std::cerr << "Error loading shop card pool: " << error << std::endl;
    }

    cardRewardSystem.ApplyOwnedRewards(cardSystem);
    rewardPickedAfterWin = false;
    shopOpenedAfterWin = false;

    cardSystem.ShuffleDeck();

    // Start the game with 5 cards in hand
    cardSystem.DealNewHand(5, objectsList);

    // Card system UI (discard/draw pile buttons + drop zones)
    cardSystem.InitUI(objectsList);

    {
        skipTurnButton.InitPreset(Button::Preset::SkipTurn, objectsList);
    }

    {
        skipTurnHintText = new TextObject();
        SDL_Color hintColor = { 245, 245, 245, 255 };
        skipTurnHintText->LoadText("End turn without draw card", hintColor, 22);
        skipTurnHintText->SetPosition(Button::HiddenHintPosition());
        objectsList.push_back(skipTurnHintText);
    }

    {
        viewDeckHintText = new TextObject();
        SDL_Color hintColor = { 245, 245, 245, 255 };
        viewDeckHintText->LoadText("View all the cards in your deck", hintColor, 22);
        viewDeckHintText->SetPosition(Button::HiddenHintPosition());
        objectsList.push_back(viewDeckHintText);
    }

    {
        gameOverText = new TextObject();
        SDL_Color color = { 255, 255, 255 };

        gameOverText->LoadText("GAME OVER", color, 80);
        //gameOverText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        gameOverText->SetPosition(glm::vec3(0.0f, 10000.0f, 10.0f));

        objectsList.push_back(gameOverText);
    }

    {
		winText = new TextObject();
        SDL_Color color = { 255, 255, 255 };
        winText->LoadText("YOU WIN!", color, 80);
        winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        winText->SetPosition(glm::vec3(0.0f, 10000.0f, 10.0f));
		objectsList.push_back(winText);
    }

    std::cout << "Init Level" << std::endl;
}

void Level::LevelUpdate()
{
    anyEnemyDied = false;

    int deltaTime = GameEngine::GetInstance()->GetDeltaTime();

    // ---- Update floating damage popups ----
    for (auto& popup : damagePopups)
    {
        if (popup.expired) continue;

        popup.timer += deltaTime;
        float progress = popup.timer / popup.duration; // 0..1

        // Float upward
        glm::vec3 p = popup.text->GetPosition();
        p.y += popup.floatSpeed * deltaTime;
        popup.text->SetPosition(p);

        // Fade out (ease-in: stay solid then fade quickly at the end)
        float fadeAlpha = 1.0f - (progress * progress);
        popup.text->SetAlpha(std::max(0.0f, fadeAlpha));

        if (popup.timer >= popup.duration)
        {
            popup.expired = true;
            // Hide the text by zeroing its size so it doesn't render
            popup.text->SetSize(0.0f, 0.0f);
        }
    }

    // Remove expired popups from the vector (text stays in objectsList and gets
    // deleted with everything else in LevelFree)
    damagePopups.erase(
        std::remove_if(damagePopups.begin(), damagePopups.end(),
            [](const DamagePopup& p) { return p.expired; }),
        damagePopups.end()
    );
    if (playerPlayingOneShot)
    {
        playerAnimTimer += deltaTime;

        if (playerAnimTimer >= playerAnimDuration)
        {
            playerPlayingOneShot = false;

            if (isGameOver==true)
            {
                if (direction == 0) playersprite->SetAnimationLoop(7, 4, 0, 1000);
                if (direction == 1) playersprite->SetAnimationLoop(7, 9, 0, 1000);
                if (direction == 2) playersprite->SetAnimationLoop(8, 4, 0, 1000);
                if (direction == 3) playersprite->SetAnimationLoop(8, 9, 0, 1000);
            }
            else
            {
                playerState = PlayerState::IDLE;
                UpdatePlayerAnimation();
            }
        }
    }
    if (!isGameOver && playerHealth <= 0)
    {
        HandlePlayerDeath();
    }

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
        bool moveBlocked = false;
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            if (e->getNowRow() == targetRow && e->getNowCol() == targetCol)
            {
                if (playerData.GetJumpCharges() > 0)
                {
                    playerData.ConsumeJumpCharge();
                    std::cout << "[Jump] Passing through enemy at ("
                        << targetRow << "," << targetCol << ")! Charges left: "
                        << playerData.GetJumpCharges() << "\n";
                }
                else
                {
                    moveBlocked = true;
                }
                break;
            }
        }
        if (moveBlocked)
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

        glm::vec3 newPos = playerMoveStart + (playerMoveTarget - playerMoveStart) * t;

        playerData.SetPosition(newPos);

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
            highlightManager.HideEnemyAttack(enemyHighlightIndex);

            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getObject()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getHPText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCorruptText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getDebuffText()), objectsList.end());

            delete e;
            it = enemies.erase(it);

            anyEnemyDied = true;
        }
        else
        {
            ++it;
        }
    }
    if (anyEnemyDied)
    {
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;

            if (EnemyCanAttackPlayer(e))
            {
                e->setPreparingAttack(true);
            }
            else
            {
                e->setPreparingAttack(false);
            }
        }
        PreviewAllEnemyAttacks();
    }
    if(enemies.empty() && !isGameOver)
    {
        if (winText)
        {
            winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        }
        turnState = TurnState::GAME_OVER;

        // Only open the card reward if the shop is NOT open (shop is opened via 'o')
        if (!rewardPickedAfterWin && !shopSystem.IsActive() && !shopOpenedAfterWin)
        {
            cardRewardSystem.Open(objectsList);
        }
	}
    
}

void Level::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void Level::LevelFree()
{
    cardInspect.Hide(objectsList);

    if (deckViewer.IsActive())
    {
        deckViewer.Hide(objectsList);
    }

    cardRewardSystem.Close(objectsList);
    shopSystem.Close(objectsList);

    // 1. Clear card system (removes card layers from objectsList, nulls its own pointers)
    cardSystem.Clear(objectsList);

    // 2. Remove enemy-owned objects from objectsList before deleting enemy,
    //    to avoid double-free when the objectsList loop runs.
    for (auto* e : enemies)
    {
        if (!e)
        {
            continue;
        }

        SpriteObject* obj = e->getObject();
        if (obj)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end()) objectsList.erase(it);
        }

        TextObject* hp = e->getHPText();
        if (hp)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), hp);
            if (it != objectsList.end()) objectsList.erase(it);
        }

        TextObject* cor = e->getCorruptText();
        if (cor)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), cor);
            if (it != objectsList.end()) objectsList.erase(it);
        }

        TextObject* deb = e->getDebuffText();
        if (deb)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), deb);
            if (it != objectsList.end()) objectsList.erase(it);
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
    nowRow = startRow;
    nowCol = startCol;
    turnCount = 0;
    playerHealth = 5;
    maxPlayerHealth = 10;
    playerData = Player();
    isGameOver = false;
    playerDead = false;
    anyEnemyDied = false;
    enemyActing = false;
    currentEnemyIndex = 0;
    currentPatternIndex = 0;
    currentRotation = 0;
    playerPlayingOneShot = false;
    playerAnimTimer = 0.0f;
    playerAnimDuration = 0.0f;
    playerMoving = false;
    playerAttacking = false;
    playerMoveTimer = 0.0f;
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
    skipTurnHintText = nullptr;
    viewDeckHintText = nullptr;
    rewardPickedAfterWin = false;
    shopOpenedAfterWin = false;
    viewDeckButton.Reset();
    skipTurnButton.Reset();

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
    if (key == 'r')
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART;
        return;
    }

    if (key == 'q')
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
        return;
    }

    if (cardRewardSystem.IsActive())
    {
        if (cardRewardSystem.HandleKeySelection(key, cardSystem, objectsList))
        {
            rewardPickedAfterWin = true;
            return;
        }

        return;
    }

    // Toggle deck viewer with 'v' key
    if (key == 'v')
    {
        cardInspect.Hide(objectsList);

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
        cardInspect.Hide(objectsList);

        if (deckViewer.IsActive()) {
            deckViewer.Hide(objectsList);
        }

        for (Enemy* e : enemies) {
            if (!e) {
                continue;
            }

            highlightManager.HideEnemyAttack(e->highlightIndex);

            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getObject()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getHPText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCorruptText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getDebuffText()), objectsList.end());

            delete e;
        }

        enemies.clear();
        enemyActing = false;
        currentEnemyIndex = 0;

        if (winText) {
            winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        }

        turnState = TurnState::GAME_OVER;

        if (!rewardPickedAfterWin) {
            cardRewardSystem.Open(objectsList);
        }

        return;
    }
    if(key == 'l'){
	}

    if (key == 'o') {
        // Debug: instant win → open shop
        cardInspect.Hide(objectsList);
        if (deckViewer.IsActive()) deckViewer.Hide(objectsList);

        for (Enemy* e : enemies)
        {
            if (!e) continue;
            highlightManager.HideEnemyAttack(e->highlightIndex);
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getObject()),     objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getHPText()),     objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCorruptText()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getDebuffText()), objectsList.end());
            delete e;
        }
        enemies.clear();
        enemyActing = false;
        currentEnemyIndex = 0;

        if (winText)
            winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));

        turnState = TurnState::GAME_OVER;

        if (!shopOpenedAfterWin)
        {
            shopSystem.Open(objectsList, playerData);
            shopOpenedAfterWin = true;
        }
        return;
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

    playerMoveStart = playerData.GetPosition();
    playerMoveTarget = GridToWorld(targetRow, targetCol);

    playerData.SetPlayerWalk(ConvertDir(playerDir));

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

    if (shopSystem.IsActive())
    {
        if (type == 0)
        {
            shopSystem.HandleMouseClick(mousePos, cardSystem, playerData, objectsList);
        }
        return;
    }

    if (cardRewardSystem.IsActive())
    {
        if (type == 0)
        {
            if (cardRewardSystem.HandleMouseClick(mousePos, cardSystem, objectsList))
            {
                rewardPickedAfterWin = true;
            }
        }

        return;
    }

    if (skipTurnHintText)
    {
        if (skipTurnButton.IsClicked(realX, realY) && turnState == TurnState::PLAYER_TURN)
            skipTurnHintText->SetPosition(skipTurnButton.GetHintPosition());
        else
            skipTurnHintText->SetPosition(Button::HiddenHintPosition());
    }

    if (viewDeckHintText)
    {
        if (viewDeckButton.IsClicked(realX, realY))
            viewDeckHintText->SetPosition(viewDeckButton.GetHintPosition());
        else
            viewDeckHintText->SetPosition(Button::HiddenHintPosition());
    }

    // ViewDeck button 
    if (type == 0 && viewDeckButton.IsClicked(realX, realY))
    {
        cardInspect.Hide(objectsList);

        if (deckViewer.IsActive())
        {
            deckViewer.Hide(objectsList);
        }
        else
        {
            const vector<Card*>& allCards = cardSystem.GetFullCollection();
            deckViewer.SetDeck(allCards);
            deckViewer.Show(objectsList);
        }
        return;
    }

    if (cardInspect.IsVisible() && type != 4)
    {
        cardSystem.UpdateHover(mousePos, true, objectsList);
        return;
    }

    // ----------------------------------------------------
    // Right-Click Down (type 4)
    // ----------------------------------------------------
    if (type == 4)
    {
        if (cardSystem.IsDrawPileClicked(mousePos))
        {
            cardInspect.Hide(objectsList);
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
            cardInspect.Hide(objectsList);
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
            Card* cardData = deckViewer.PeekAt(mousePos);
            if (cardData)
            {
                if (cardInspect.IsInspecting(cardData)) {
                    cardInspect.Hide(objectsList);
                }
                else {
                    cardInspect.Show(cardData, cardSystem, objectsList);
                }
            }
            else
            {
                cardInspect.Hide(objectsList);
            }
            return;
        }

        ImageObject* hit = cardSystem.PeekAt(mousePos);
        Card* cardData = hit ? cardSystem.FindCardByImage(hit) : nullptr;

        if (cardData)
        {
            if (cardInspect.IsInspecting(cardData)) {
                cardInspect.Hide(objectsList);
            }
            else {
                cardInspect.Show(cardData, cardSystem, objectsList);
            }
        }
        else
        {
            cardInspect.Hide(objectsList);
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
        if (type == 0)
        {
            if (deckViewer.HandleClick(mousePos, objectsList))
                return;
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

        // SKIP TURN BUTTON 
        if (skipTurnButton.IsClicked(realX, realY))
        {
            cardSystem.UpdateHover(mousePos, false, objectsList);
            highlightManager.HideAllPlayer();
            turnState = TurnState::ENEMY_TURN;
            return;
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

                    // Build context and execute all card actions via OOP dispatch
                    CardPlayContext ctx {
                        playerData, enemies, cardSystem, objectsList,
                        dz, nowRow, nowCol,
                        GridStartRow, GridEndRow, GridStartCol, GridEndCol
                    };

                    CardPlayResult result = CardActionExecutor::ExecuteCard(cardData, ctx);

                    // Apply attack patterns to the grid (damage + debuffs)
                    CardActionExecutor::ApplyAttackPatterns(result, ctx);

                    // Spawn a floating damage number for every hit
                    // Multi-hit cards (repeatCount > 1) push one HitInfo per repeat,
                    // staggered upward so they fan out instead of stacking
                    for (const HitInfo& hit : result.hits)
                    {
                        glm::vec3 hitWorld = GridToWorld(hit.row, hit.col);
                        hitWorld.y += hit.repeatIndex * 22.0f; // stagger per repeat
                        SpawnDamagePopup(hitWorld, hit.damage);
                    }

                    // --- Animation & turn state (stays in Level) ---
                    int retreatDir = CardActionExecutor::GetRetreatDirection(dz);
                    int totalMoveSteps = result.moveSteps + result.retreatSteps;
                    int moveDir = dz;
                    if (result.retreatSteps > 0 && result.moveSteps == 0)
                    {
                        moveDir = retreatDir;
                    }

                    pendingFastCard = result.isFastCard;

                    if (result.hasAttack())
                    {
                        playerState = PlayerState::ATTACK;
                        UpdatePlayerAnimation();

                        playerAttacking = true;
                        attackTimer = 0;

                        std::cout << "[Attack Animation Started]\n";

                        if (result.retreatSteps > 0)
                        {
                            pendingMoveSteps = result.retreatSteps;
                            pendingMoveZone = retreatDir;
                        }
                        else
                        {
                            pendingMoveSteps = result.moveSteps;
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

                        if (!result.isFastCard)
                        {
                            tempDiscardDone = false;
                            turnState = TurnState::ENEMY_TURN;
                        }
                    }

                    // Handle del (delete) vs normal discard
                    if (result.isDeleteAfterUse)
                    {
                        std::cout << "[Del] Card moved to delete pile.\n";
                        cardSystem.EndDragConfirmDelete(dragCard, objectsList);
                    }
                    else
                    {
                        cardSystem.EndDragConfirm(dragCard, objectsList);
                    }

                    if (result.isFastCard)
                    {
                        std::cout << "[Fast] Will not consume player turn.\n";
                    }

                    if (result.isLagCard)
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
    if (!e || e->getIsDead())
    {
        return;
    }

    if (!playersprite)
    {
        return;
    }

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
            PlayerTakeDamage(e->getAttackDamage());
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

        if (!enemyActing)
        {
            if (currentEnemyIndex >= enemies.size())
            {
                currentEnemyIndex = 0;
                turnState = TurnState::END_TURN;
                return;
            }

            Enemy* e = enemies[currentEnemyIndex];

            if (!e || e->getIsDead())
            {
                currentEnemyIndex++;
                return;
            }

            enemyActing = true;

            if (e->isDelayed())
            {
                e->decrementDelay();
                enemyActing = false;
                currentEnemyIndex++;
                return;
            }

            if (e->isPreparingAttack())
            {
                ApplyEnemyAttack(e);
                highlightManager.HideEnemyAttack(e->highlightIndex);
                e->setPreparingAttack(false);

                enemyActing = false;
                currentEnemyIndex++;
                return;
            }
            else if (EnemyCanAttackPlayer(e))
            {
                e->setPreparingAttack(true);
                PreviewAllEnemyAttacks();

                enemyActing = false;
                currentEnemyIndex++;
                return;
            }
			else if (e->isPreparingAttack() == false)
            {
                int newR, newC;
                if (e->TryMoveTowardPlayer(
                    nowRow, nowCol,
                    GridStartRow, GridEndRow,
                    GridStartCol, GridEndCol,
                    enemies,
                    newR, newC))
                {
                    e->setNowPosition(newR, newC);

                    glm::vec3 world = GridToWorld(newR, newC);
                    e->StartMove(world);
                }
            }
        }

        if (currentEnemyIndex < 0 || currentEnemyIndex >= static_cast<int>(enemies.size()))
        {
            enemyActing = false;
            currentEnemyIndex = 0;
            turnState = TurnState::END_TURN;
            return;
        }

        Enemy* e = enemies[currentEnemyIndex];
        if (e && !e->getIsMoving())
        {
            enemyActing = false;
            currentEnemyIndex++;
        }

        break;
    }

    case TurnState::END_TURN:
    {
        highlightManager.HideEnemyAttack(enemyHighlightIndex);

        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            e->setPreparingAttack(false);
        e->decrementWeaken();
        }
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;

            if (EnemyCanAttackPlayer(e))
            {
                e->setPreparingAttack(true);
            }
        }

        PreviewAllEnemyAttacks();

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
    if (!playersprite || playerPlayingOneShot) return;

    if (playerState == PlayerState::IDLE)
    {
        switch (playerDir)
        {
        case PlayerDir::DOWN:  playersprite->SetAnimationLoop(0, 0, 2, 800); direction=0; break;
        case PlayerDir::LEFT:  playersprite->SetAnimationLoop(0, 2, 2, 800); direction = 1; break;
        case PlayerDir::UP:    playersprite->SetAnimationLoop(0, 4, 2, 800); direction = 2; break;
        case PlayerDir::RIGHT: playersprite->SetAnimationLoop(0, 6, 2, 800); direction = 3; break;
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
        case PlayerDir::DOWN:  playersprite->SetAnimationOnce(2, 0, 8, 100); break;
        case PlayerDir::LEFT:  playersprite->SetAnimationOnce(2, 8, 8, 100); break;
        case PlayerDir::UP:    playersprite->SetAnimationOnce(3, 0, 8, 100); break;
        case PlayerDir::RIGHT: playersprite->SetAnimationOnce(3, 8, 8, 100); break;
        }
    }
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

/*void Level::PreviewEnemyAttack(Enemy* e)
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
}*/
void Level::PreviewAllEnemyAttacks()
{
    highlightManager.HideEnemyAttack(enemyHighlightIndex);

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        if (!e->isPreparingAttack()) continue;

        auto cells = e->getCurrentPattern().applyTo(
            e->getNowRow(),
            e->getNowCol()
        );

        highlightManager.ShowEnemyAttack(
            cells,
            GridStartRow, GridEndRow,
            GridStartCol, GridEndCol,
            [this](int r, int c) { return GridToWorld(r, c); },
            enemyHighlightIndex
        );
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

void Level::EndTurn()
{
    turnCount++;
    playerData.ResetShield();
    playerData.ExpireBarrier();
    playerData.ResetJumpCharges();
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
void Level::PlayerTakeDamage(int damage)
{
    if (playerData.HasBarrier())
    {
        std::cout << "[Barrier] Enemy damage negated while barrier is active." << std::endl;
        return;
    }

    damage = playerData.AbsorbDamage(damage);
    if (damage <= 0) return;

    playerPlayingOneShot = true;
    playerAnimTimer = 0;
    playerAnimDuration = 400;

    playerData.SetPlayerGetDamage((int)playerDir);

    playerHealth -= damage;
    UpdateHPBar();

    std::cout << "[Player] Took " << damage
        << " damage. HP: " << playerHealth << std::endl;

    if (playerHealth <= 0)
    {
        HandlePlayerDeath();
    }
}

void Level::HandlePlayerDeath()
{
    if (isGameOver) return;

    isGameOver = true;

    playerData.SetPlayerDie((int)playerDir);

    playerMoving = false;
    playerAttacking = false;

    if (gameOverText)
    {
        gameOverText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
    }

    turnState = TurnState::GAME_OVER;
}

int Level::ConvertDir(PlayerDir dir)
{
    switch (dir)
    {
    case PlayerDir::DOWN:  return 0;
    case PlayerDir::LEFT:  return 1;
    case PlayerDir::UP:    return 2;
    case PlayerDir::RIGHT: return 3;
    }
    return 0;
}

void Level::SpawnDamagePopup(glm::vec3 worldPos, int damage)
{
    DamagePopup popup;

    popup.text = new TextObject();
    SDL_Color dmgColor = { 255, 220, 40, 255 }; // bright yellow-gold
    popup.text->LoadText(std::to_string(damage), dmgColor, 32);

    // Start slightly above the centre of the hit tile
    popup.text->SetPosition(glm::vec3(worldPos.x, worldPos.y + 40.0f, 15.0f));

    popup.timer     = 0.0f;
    popup.duration  = 1200.0f; // 1.2 seconds
    popup.floatSpeed = 0.07f;  // px per ms
    popup.expired   = false;

    objectsList.push_back(popup.text);
    damagePopups.push_back(popup);
}
