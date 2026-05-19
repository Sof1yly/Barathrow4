#include "Level.h"
#include "EliteEnemy.h"
#include "EliteEnemy1.h"
#include "EliteEnemy2.h"
#include "SaveSystem.h"
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

// Data file paths 

namespace {
    constexpr const char* PATH_PATTERN        = "../Resource/GameData/Pattern.txt";
    constexpr const char* PATH_CARDS_STARTER  = "../Resource/GameData/CardActionStandard.txt";
    constexpr const char* PATH_CARDS_ALL      = "../Resource/GameData/CardAction_filled.txt";
    constexpr const char* PATH_CARD_DESC      = "../Resource/GameData/CardDesc.txt";
    constexpr const char* PATH_ENEMY_DATA     = "../Resource/GameData/EnemyData.txt";
    constexpr const char* PATH_ENEMY_PATTERN  = "../Resource/GameData/EnemyPattern.txt";
}

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
	boss = levelManager.IsBossLevel();
	srand((unsigned int)time(NULL));
	Background = new ImageObject();
	Background->SetSize(1920.0f, -1080.0f);
	Background->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	Background->SetTexture("../Resource/Texture/BG/Floor1_FHD.PNG");
	objectsList.push_back(Background);

	// Initialize player state
    playerDir = PlayerDir::DOWN;
    playerState = PlayerState::IDLE;
    UpdatePlayerAnimation();

    // 1) Tile grid
    
	for (int r = 0; r < GRID_ROWS; r++) // Initialize all tiles as walkable
    {
        for (int c = 0; c < GRID_COLS; c++)
        {
            walkable[r][c] = true;
        }
    }
    
	if (boss) { //make a non-walkable area for boss level
        for (int i = 0; i < 2; i++) {
            for (int j = 2; j < 7; j++) {
                walkable[j][i] = false;
            }
        }
    }
    
    for (int i = GridStartRow; i < GridEndRow; ++i) //Render
    {
        for (int j = GridStartCol; j < GridEndCol; ++j)
        {
            if (!walkable[i][j])
                continue;

            ImageObject* tile = new ImageObject();
            tile->SetTexture("../Resource/Texture/BG/F1Grid.png");
            tile->SetSize(GridWide, GridHigh);
            tile->SetPosition(glm::vec3(
                i * 101.0f - 404.0f,
                j * -105.0f + 352.0f,
                0.0f));

            objectsList.push_back(tile);
            gridTiles.push_back(tile);
        }
    }

    highlightManager.Init(objectsList, GridWide, GridHigh);


    if (boss)
        InitBossLevel();

    // Load save data early so player position and enemy layout are correct from the start
    SaveData pendingSave;
    bool hasSave = false;
    if (SaveSystem::pendingLoad)
    {
        hasSave = SaveSystem::Load(pendingSave);
        if (hasSave)
        {
            nowRow              = pendingSave.playerRow;
            nowCol              = pendingSave.playerCol;
            levelManager.SetLevel(pendingSave.currentLevel);
            baseHandSize        = pendingSave.baseHandSize;
            goldBonusActive     = pendingSave.goldBonusActive;
            startCombatBarrier  = pendingSave.startCombatBarrier;
            startCombatOverclock = pendingSave.startCombatOverclock;
            eventSceneDone      = pendingSave.eventSceneDone;
        }
    }

    // Determine elite flags before setting spawn position so SetPlayerSpawnPosition uses them
    {
        LevelConfig cfg = levelManager.GetCurrentConfig();
        elite1 = (cfg.type == LevelConfig::Type::Elite1);
        elite2 = (cfg.type == LevelConfig::Type::Elite2);
        if (cfg.type == LevelConfig::Type::EliteRandom)
        {
            if (rand() % 2 == 0) elite1 = true;
            else                  elite2 = true;
        }
    }

    if (!hasSave)
        SetPlayerSpawnPosition();

    LoadEnemyData();

    if (hasSave)
        RestoreEnemiesFromSave(pendingSave);
    else
        SpawnEnemiesForLevel();


    // 3) Player sprite (3x4, 192x256)
    {
        glm::vec3 startPos = GridToWorld(nowRow, nowCol);
        playerData.InitSprite(objectsList, startPos);
        playersprite = playerData.GetSprite();
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
        viewMapIcon = new ImageObject();
        viewMapIcon->SetSize(80.0f, -80.0f);
        viewMapIcon->SetPosition(glm::vec3(805.0f, 500.0f, 0.0f));
        viewMapIcon->SetTexture("../Resource/Texture/UI/Info.PNG");
        objectsList.push_back(viewMapIcon);
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
    if (!cardSystem.LoadData(PATH_PATTERN, PATH_CARDS_STARTER, PATH_CARD_DESC, &error)) {
        std::cerr << "Error loading card data: " << error << std::endl;
    }

    if (!cardRewardSystem.LoadPoolData(PATH_PATTERN, PATH_CARDS_ALL, PATH_CARD_DESC, &error)) {
        std::cerr << "Error loading reward card pool: " << error << std::endl;
    }

    if (!shopSystem.LoadPoolData(PATH_PATTERN, PATH_CARDS_ALL, PATH_CARD_DESC, &error)) {
        std::cerr << "Error loading shop card pool: " << error << std::endl;
    }

    rewardPickedAfterWin = false;
    shopOpenedAfterWin   = false;
    bool pendingRemoveCards = false;

    if (hasSave)
    {
        // Position, level, and run-effects were already applied early in LevelInit.
        // Finish restoring player stats, then cards.
        playerData.setHp(pendingSave.playerHp);
        playerData.setMaxHp(pendingSave.playerMaxHp);
        playerData.SetCoins(pendingSave.playerCoins);
        playerData.SetBarrierCount(pendingSave.playerBarrierCount);
        playerData.SetJumpCharges(pendingSave.playerJumpCharges);
        playerData.SetPosition(GridToWorld(nowRow, nowCol));

        // Restore deck and hand
        std::vector<Card*> owned;
        cardSystem.RebuildDeckFromSave(pendingSave.cardNames, cardRewardSystem.GetRewardLoader(), owned);
        cardSystem.InitUI(objectsList);
        cardSystem.DealSavedHand(pendingSave.handCardNames, baseHandSize, objectsList);

        UpdateHPBar();
        if (levelText)
        {
            SDL_Color col = { 255, 230, 100, 255 };
            levelText->LoadText(levelManager.GetLevelText(), col, 30);
        }
        SaveSystem::pendingLoad = false;
    }
    else
    {
        // Normal new-game path
        cardRewardSystem.ApplyOwnedRewards(cardSystem);

        if (GameData::GetInstance()->eventEffectType >= 0)
        {
            EventScene::EffectType eff = static_cast<EventScene::EffectType>(GameData::GetInstance()->eventEffectType);
            GameData::GetInstance()->eventEffectType = -1;

            if (eff == EventScene::EffectType::REMOVE_CARDS)
            {
                pendingRemoveCards = true;
            }
            else
            {
                eventSceneDone = true;
                ApplyEventEffect(eff);
            }
        }

        cardSystem.ShuffleDeck();

        if (eventSceneDone)
            cardSystem.DealNewHand(baseHandSize, objectsList);

        cardSystem.InitUI(objectsList);
    }

    // Apply start-of-combat buffs from event effects (mirrors AdvanceToNextRound)
    if (startCombatBarrier   > 0) playerData.AddBarrier(startCombatBarrier);
    if (startCombatOverclock > 0) cardSystem.ApplyOverclock(startCombatOverclock);

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
        viewMapHintText = new TextObject();
        SDL_Color hintColor = { 245, 245, 245, 255 };
        viewMapHintText->LoadText("View the level map", hintColor, 22);
        viewMapHintText->SetPosition(Button::HiddenHintPosition());
        objectsList.push_back(viewMapHintText);
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

    {
        levelNameBanner = new TextObject();
        SDL_Color color = { 255, 230, 100, 255 };
        levelNameBanner->LoadText(levelManager.GetLevelText(), color, 80);
        levelNameBanner->SetPosition(glm::vec3(0.0f, 10000.0f, 11.0f));
        objectsList.push_back(levelNameBanner);
    }

    {
        levelText = new TextObject();
        SDL_Color color = { 255, 230, 100, 255 };
        levelText->LoadText(levelManager.GetLevelText(), color, 30);
        levelText->SetPosition(glm::vec3(750.0f, 460.0f, 10.0f));
        objectsList.push_back(levelText);
    }

    {
        speedBtnIcon = new ImageObject();
        speedBtnIcon->SetTexture("../Resource/Texture/UI/f1.png");
        speedBtnIcon->SetSize(60.0f, -60.0f);
        speedBtnIcon->SetPosition(glm::vec3(-900.0f, 310.0f, 10.0f));
        objectsList.push_back(speedBtnIcon);

        speedBtnText = new TextObject();
        SDL_Color c = { 255, 255, 255, 255 };
        speedBtnText->LoadText("1x", c, 22);
        speedBtnText->SetPosition(glm::vec3(-855.0f, 307.0f, 10.0f));
        objectsList.push_back(speedBtnText);
    }

    // Open remove scene last so its overlay renders on top of all other UI
    if (pendingRemoveCards)
    {
        ApplyEventEffect(EventScene::EffectType::REMOVE_CARDS);
    }

    settingPage.Init(objectsList);
    pauseMenu.Init(objectsList);

    std::cout << "Init Level" << std::endl;
}

void Level::LevelUpdate()
{
    anyEnemyDied = false;

    int deltaTime = GameEngine::GetInstance()->GetDeltaTime();
    if (fastMode) deltaTime *= 2;

    if (pauseMenuActive)
        return;

    if (settingPageActive)
    {
        for (auto* obj : objectsList)
            obj->Update((float)deltaTime);
        return;
    }

    if (eventRemoveScene.IsActive())
    {
        for (auto* obj : objectsList)
            obj->Update((float)deltaTime);
        return;
    }

    // Map scene (win transition or view-only): update objects then run state machine
    if (mapSceneActive)
    {
        for (auto* obj : objectsList)
            obj->Update((float)deltaTime);
        mapScene.Update((float)deltaTime);
        if (mapScene.IsDone())
        {
            bool wasViewOnly = mapScene.IsViewOnly();
            mapScene.Close(objectsList);
            mapSceneActive = false;
            if (!wasViewOnly)
                ResetForNextCombat();
        }
        return;
    }

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

    // ---- Update Elite1 projectiles ----
    for (auto& proj : elite1Projectiles)
    {
        if (proj.done) continue;

        proj.timer += deltaTime;
        float t = proj.timer / proj.duration;
        if (t >= 1.0f) t = 1.0f;

        glm::vec3 pos = proj.startPos + (proj.endPos - proj.startPos) * t;
        pos.z = 6.0f;
        proj.sprite->SetPosition(pos);
        proj.sprite->Update(deltaTime);

        if (t >= 1.0f)
        {
            proj.done = true;
            proj.sprite->SetSize(0.0f, 0.0f); // hide at destination
        }
    }
    elite1Projectiles.erase(
        std::remove_if(elite1Projectiles.begin(), elite1Projectiles.end(),
            [](const Elite1Projectile& p) { return p.done; }),
        elite1Projectiles.end()
    );

    // ---- Update Elite2 falling projectiles ----
    for (auto& proj : elite2Projectiles)
    {
        if (proj.done) continue;

        proj.timer += deltaTime;
        float t = proj.timer / proj.duration;
        if (t >= 1.0f) t = 1.0f;

        // ease-in: accelerates as it falls (t^2)
        float et = t * t;
        glm::vec3 pos = proj.startPos + (proj.endPos - proj.startPos) * et;
        pos.z = 6.0f;
        proj.sprite->SetPosition(pos);
        proj.sprite->Update(deltaTime);

        if (t >= 1.0f)
        {
            proj.done = true;
            proj.sprite->SetSize(0.0f, 0.0f);
        }
    }
    elite2Projectiles.erase(
        std::remove_if(elite2Projectiles.begin(), elite2Projectiles.end(),
            [](const Elite2Projectile& p) { return p.done; }),
        elite2Projectiles.end()
    );

    // ---- Update BossAttack1 tile-flash effects ----
    // Animation is already ticked by the objectsList update below; just watch for finish.
    for (auto& fx : bossAttack1Effects)
    {
        if (fx.done) continue;
        if (fx.sprite && fx.sprite->IsFinished())
        {
            fx.done = true;
            fx.sprite->SetSize(0.0f, 0.0f);
        }
    }
    bossAttack1Effects.erase(
        std::remove_if(bossAttack1Effects.begin(), bossAttack1Effects.end(),
            [](const BossAttack1Effect& f) { return f.done; }),
        bossAttack1Effects.end()
    );

    // ---- Update BossAttack2 travelling projectiles ----
    for (auto& proj : bossAttack2Projectiles)
    {
        if (proj.done) continue;

        proj.timer += deltaTime;
        float t = std::min(proj.timer / proj.duration, 1.0f);

        glm::vec3 pos = proj.startPos + (proj.endPos - proj.startPos) * t;
        pos.z = 6.0f;
        proj.sprite->SetPosition(pos);
        proj.sprite->Update(deltaTime);

        if (t >= 1.0f)
        {
            proj.done = true;
            proj.sprite->SetSize(0.0f, 0.0f);
        }
    }
    bossAttack2Projectiles.erase(
        std::remove_if(bossAttack2Projectiles.begin(), bossAttack2Projectiles.end(),
            [](const BossAttack2Projectile& p) { return p.done; }),
        bossAttack2Projectiles.end()
    );

    // ---- Update boss summon portals ----
    for (auto& portal : bossSummonPortals)
    {
        if (portal.done) continue;

        portal.timer += deltaTime;

        // On the 4th frame (after 3 × 150 ms), spawn the enemy
        if (!portal.enemySpawned && portal.timer >= portal.spawnDelay)
        {
            Enemy* ne = new Enemy(portal.enemyType);
            ne->setNowPosition(portal.spawnRow, portal.spawnCol);
            ne->SetWorldPosition(GridToWorld(portal.spawnRow, portal.spawnCol));
            enemies.push_back(ne);
            objectsList.push_back(ne->getObject());
            objectsList.push_back(ne->getHPText());
            objectsList.push_back(ne->getCorruptText());
            objectsList.push_back(ne->getDebuffText());
            objectsList.push_back(ne->getCountdownIcon());
            objectsList.push_back(ne->getCountdownText());
            portal.enemySpawned = true;
            std::cout << "[Boss] Summoned enemy at ("
                      << portal.spawnRow << ", " << portal.spawnCol << ")\n";
        }

        // Portal disappears when animation finishes (6 frames × 300 ms = 1800 ms)
        if (portal.sprite && portal.sprite->IsFinished())
        {
            portal.done = true;
            portal.sprite->SetSize(0.0f, 0.0f);
        }
    }
    bossSummonPortals.erase(
        std::remove_if(bossSummonPortals.begin(), bossSummonPortals.end(),
            [](const BossSummonPortal& p) { return p.done; }),
        bossSummonPortals.end()
    );

    if (winDelayActive)
    {
        winDelay += deltaTime;
        if (winDelay >= 2500.0f)
        {
            winDelayActive = false;
            rewardBoxScene.Open(pendingCoinsEarned, objectsList);
            rewardPickedAfterWin = true;
        }
    }

    if (levelBannerActive)
    {
        levelBannerTimer += deltaTime;
        if (levelBannerTimer >= 2000.0f)
        {
            levelBannerActive = false;
            if (levelNameBanner) levelNameBanner->SetPosition(glm::vec3(0.0f, 10000.0f, 11.0f));
        }
    }

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
    if (!isGameOver && playerData.getHp() <= 0)
    {
        HandlePlayerDeath();
    }

    // Battery spawn trigger: fires once when boss HP drops to ≤50%
    if (boss && bossEnemy && !bossEnemy->getIsDead() && bossEnemy->ShouldSpawnBatteries())
        SpawnBatteries();

    // Keep boss shield flag in sync: blocks player damage while any battery is alive
    if (boss && bossEnemy && !bossEnemy->getIsDead())
        bossEnemy->setShieldedFromPlayer(AnyBatteryAlive());

    UpdateTurn();
    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;

        if (playersprite)
            if (EliteEnemy* elite = dynamic_cast<EliteEnemy*>(e))
                elite->UpdateFacing(playersprite->GetPosition());

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
        if (!IsWalkable(targetRow, targetCol))
        {
            std::cout << "[Move Blocked] Void tile.\n";

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
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead() || e->getHealth() <= 0) continue;
            if (dynamic_cast<EliteEnemy1*>(e) && e->getNowRow() == targetRow)
            {
                std::cout << "[Move Blocked] Elite1 guards this edge row.\n";
                pendingMoveSteps = 0;
                playerMoving = false;
                playerState = PlayerState::IDLE;
                UpdatePlayerAnimation();
                if (pendingFastCard) { turnState = TurnState::PLAYER_TURN; pendingFastCard = false; }
                else                 { turnState = TurnState::ENEMY_TURN; }
                return;
            }
        }
        bool moveBlocked = false;
        for (auto* e : enemies)
        {
            if (!e || e->getIsDead()) continue;
            if (e->OccupiesTile(targetRow, targetCol))
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
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCountdownIcon()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCountdownText()), objectsList.end());

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

            if (e->isPreparingAttack())
            {
                // Enemy was mid-countdown: keep it preparing but refresh the locked
                // position so highlights and damage land in the right place.
                if (EnemyCanAttackPlayer(e))
                    e->LockAttackPattern(nowRow, nowCol);
                else
                    e->setPreparingAttack(false); // can no longer reach player
            }
            // Enemies that are NOT preparing are intentionally left alone.
            // UpdateTurn will arm them (with the correct countdown) on their next turn.
            // Re-arming them here without setCountDownR() causes double damage because
            // countdownRemaining stays at 0 and they fire again immediately next turn.
        }
        PreviewAllEnemyAttacks();
    }
    if(enemies.empty() && !isGameOver && !inShopOnlyLevel)
    {
        if (winText)
        {
            winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        }
        turnState = TurnState::GAME_OVER;

        // Trigger once: start 2-second delay before opening reward box
        if (!rewardPickedAfterWin && !winDelayActive)
        {
            pendingCoinsEarned = levelManager.RollCoins();
            if (goldBonusActive) pendingCoinsEarned = pendingCoinsEarned * 5 / 4;
            winDelay = 0.0f;
            winDelayActive = true;
        }
	}
    
}

void Level::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void Level::LevelFree()
{
    if (pauseMenuActive)
    {
        pauseMenu.Hide();
        pauseMenuActive = false;
    }
    pauseMenu.Reset();

    if (settingPageActive)
    {
        settingPage.Hide();
        settingPageActive = false;
    }
    settingPage.Reset();

    if (mapSceneActive)
    {
        mapScene.Close(objectsList);
        mapSceneActive = false;
    }

    eventScene.Close(objectsList);
    eventRemoveScene.Close(objectsList);
    cardInspect.Hide(objectsList);

    if (deckViewer.IsActive())
    {
        deckViewer.Hide(objectsList);
    }

    cardRewardSystem.Close(objectsList);
    shopSystem.Close(objectsList);
    rewardBoxScene.Close(objectsList);

    // 1. Clear card system (removes card layers from objectsList, nulls its own pointers)
    cardSystem.Clear(objectsList);

    // Reset subsystems that hold persistent run state so a restart starts clean
    cardRewardSystem.Reset();
    shopSystem.Reset();

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

        ImageObject* cdi = e->getCountdownIcon();
        if (cdi)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), cdi);
            if (it != objectsList.end()) objectsList.erase(it);
        }

        TextObject* cdt = e->getCountdownText();
        if (cdt)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), cdt);
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
    damagePopups.clear();

    // Projectile sprites were already freed above via objectsList — just drop the structs.
    // Leaving these vectors non-empty causes a dangling-pointer crash on the next LevelUpdate.
    elite1Projectiles.clear();
    elite2Projectiles.clear();
    bossAttack1Effects.clear();
    bossAttack2Projectiles.clear();
    bossSummonPortals.clear();
    nowRow = startRow;
    nowCol = startCol;
    turnCount = 0;
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
    viewMapHintText  = nullptr;
    rewardPickedAfterWin = false;
    shopOpenedAfterWin = false;
    inShopOnlyLevel = false;
    levelManager.Reset();
    gridTiles.clear();
    Background = nullptr;
    levelText = nullptr;
    fastMode = false;
    speedBtnIcon = nullptr;
    speedBtnText = nullptr;
    viewDeckButton.Reset();
    skipTurnButton.Reset();
    viewMapIcon = nullptr;
    eventSceneDone        = false;
    baseHandSize          = 5;
    goldBonusActive       = false;
    startCombatBarrier    = 0;
    startCombatOverclock  = 0;

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

    if (key == 'o')
    {
        levelManager.SetLevel(17); // SetLevel(17) so Advance() brings it to 18
        AdvanceToNextRound();
        return;
    }

    if (key == 'f')
    {
        fastMode = !fastMode;
        if (speedBtnIcon)
            speedBtnIcon->SetTexture(fastMode
                ? "../Resource/Texture/UI/f2.png"
                : "../Resource/Texture/UI/f1.png");
        if (speedBtnText)
        {
            SDL_Color c = { 255, 255, 255, 255 };
            speedBtnText->LoadText(fastMode ? "2x" : "1x", c, 22);
            speedBtnText->SetPosition(glm::vec3(-855.0f, 307.0f, 10.0f));
        }
        return;
    }

    if (mapSceneActive)
    {
        // In view-only mode any key (except q/r/f already handled above) closes the map
        if (mapScene.IsViewOnly())
            mapScene.HandleClose();
        return;
    }

    if (eventScene.IsActive() || eventRemoveScene.IsActive()) return;

    if (shopSystem.IsActive()) return;

    if (rewardBoxScene.IsActive() && !cardRewardSystem.IsActive()) return;

    if (cardRewardSystem.IsActive())
    {
        if (cardRewardSystem.HandleKeySelection(key, cardSystem, objectsList))
        {
            rewardBoxScene.ClaimCard();
            if (rewardBoxScene.IsDone())
                AdvanceToNextRound();
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
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCountdownIcon()), objectsList.end());
            objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), e->getCountdownText()), objectsList.end());

            delete e;
        }

        enemies.clear();
        enemyActing = false;
        currentEnemyIndex = 0;

        if (winText) {
            winText->SetPosition(glm::vec3(0.0f, 100.0f, 10.0f));
        }

        turnState = TurnState::GAME_OVER;

        if (!rewardPickedAfterWin && !winDelayActive)
        {
            pendingCoinsEarned = levelManager.RollCoins();
            if (goldBonusActive) pendingCoinsEarned = pendingCoinsEarned * 5 / 4;
            winDelay = 0.0f;
            winDelayActive = true;
        }

        return;
    }
    if(key == 'l'){
	}

    // if (key == 'o') { /* debug: instant win → open shop — disabled */ }

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

    if (!IsWalkable(targetRow, targetCol))
    {
        std::cout << "[Blocked] Void tile.\n";
        return;
    }

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead() || e->getHealth() <= 0) continue;
        if (dynamic_cast<EliteEnemy1*>(e) && e->getNowRow() == targetRow)
        {
            std::cout << "[Blocked] Elite1 guards this edge row.\n";
            return;
        }
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

    // Route all input to the pause menu while it is open
    if (pauseMenuActive)
    {
        if (type == 3)
        {
            pauseMenu.HandleHover(realX, realY);
        }
        else if (type == 0)
        {
            auto action = pauseMenu.HandleClick(realX, realY);
            switch (action)
            {
            case PauseMenu::Action::RESUME:
                pauseMenu.Hide();
                pauseMenuActive = false;
                break;
            case PauseMenu::Action::SETTING:
                pauseMenu.Hide();
                pauseMenuActive = false;
                settingPage.Show();
                settingPageActive = true;
                break;
            case PauseMenu::Action::SAVE_QUIT_MAIN:
            case PauseMenu::Action::SAVE_QUIT_DESKTOP:
            {
                SaveData sd;
                sd.playerRow            = nowRow;
                sd.playerCol            = nowCol;
                sd.playerHp             = playerData.getHp();
                sd.playerMaxHp          = playerData.getMaxHp();
                sd.playerCoins          = playerData.GetCoins();
                sd.playerBarrierCount   = playerData.GetBarrierCount();
                sd.playerJumpCharges    = playerData.GetJumpCharges();
                sd.currentLevel         = levelManager.GetLevel();
                sd.baseHandSize         = baseHandSize;
                sd.goldBonusActive      = goldBonusActive;
                sd.startCombatBarrier   = startCombatBarrier;
                sd.startCombatOverclock = startCombatOverclock;
                sd.eventSceneDone       = eventSceneDone;
                for (Card* c : cardSystem.GetAllCards())
                    sd.cardNames.push_back(c->getName());
                for (Card* c : cardSystem.GetHand().CollectAllCardData())
                    sd.handCardNames.push_back(c->getName());

                // Serialize live enemies (skip dead ones)
                for (Enemy* e : enemies)
                {
                    if (!e || e->getIsDead()) continue;
                    EnemySaveData esd;
                    if      (dynamic_cast<Boss*>(e))        esd.typeIndex = 11;
                    else if (dynamic_cast<EliteEnemy2*>(e)) esd.typeIndex = 10;
                    else if (dynamic_cast<EliteEnemy1*>(e)) esd.typeIndex = 9;
                    else                                     esd.typeIndex = (int)e->getType();
                    esd.row              = e->getNowRow();
                    esd.col              = e->getNowCol();
                    esd.health           = e->getHealth();
                    esd.delayTurns       = e->getDelayTurns();
                    esd.corruptionStacks = e->getCorruption();
                    esd.weakenTurns      = e->getWeakenTurns();
                    sd.enemies.push_back(esd);
                }

                SaveSystem::Save(sd);

                pauseMenu.Hide();
                pauseMenuActive = false;
                auto next = (action == PauseMenu::Action::SAVE_QUIT_MAIN)
                            ? GameState::GS_MAIN_MENU
                            : GameState::GS_QUIT;
                GameData::GetInstance()->gGameStateNext = next;
                break;
            }
            case PauseMenu::Action::ABANDON:
                pauseMenu.Hide();
                pauseMenuActive = false;
                SaveSystem::DeleteSave();
                GameData::GetInstance()->gGameStateNext = GameState::GS_MAIN_MENU;
                break;
            default:
                break;
            }
        }
        return;
    }

    // Route all input to the setting page while it is open
    if (settingPageActive)
    {
        if (type == 3)
            settingPage.HandleHover(realX, realY);
        else if (type == 0)
        {
            if (settingPage.HandleClick(realX, realY) == SettingPage::Action::CLOSE)
                settingPageActive = false;
        }
        return;
    }

    // Speed toggle button — always active outside pause/setting
    {
        static const float BTN_X = -900.0f, BTN_Y = 310.0f, BTN_R = 35.0f;
        bool hit = speedBtnIcon
                && realX >= BTN_X - BTN_R && realX <= BTN_X + BTN_R
                && realY >= BTN_Y - BTN_R && realY <= BTN_Y + BTN_R;

        if (type == 3 && speedBtnIcon)
        {
            float s = hit ? 75.0f : 60.0f;
            speedBtnIcon->SetSize(s, -s);
        }
        else if (type == 0 && hit)
        {
            fastMode = !fastMode;
            speedBtnIcon->SetTexture(fastMode
                ? "../Resource/Texture/UI/f2.png"
                : "../Resource/Texture/UI/f1.png");
            if (speedBtnText)
            {
                SDL_Color c = { 255, 255, 255, 255 };
                speedBtnText->LoadText(fastMode ? "2x" : "1x", c, 22);
                speedBtnText->SetPosition(glm::vec3(-855.0f, 307.0f, 10.0f));
            }
            return;
        }
    }

    if (mapSceneActive)
    {
        if (mapScene.IsViewOnly() && type == 0)
            mapScene.HandleClick(realX, realY);
        return;
    }

    if (eventRemoveScene.IsActive())
    {
        if (type == 3)
        {
            eventRemoveScene.HandleHover(mousePos.x, mousePos.y);
        }
        else if (type == 4)
        {
            Card* evCard = eventRemoveScene.PeekCardAt(mousePos.x, mousePos.y);
            if (evCard)
            {
                if (cardInspect.IsInspecting(evCard))
                    cardInspect.Hide(objectsList);
                else
                    cardInspect.Show(evCard, cardSystem, objectsList);
            }
            else
            {
                cardInspect.Hide(objectsList);
            }
        }
        else if (type == 0)
        {
            cardInspect.Hide(objectsList);
            if (eventRemoveScene.HandleMouseClick(realX, realY, cardSystem, objectsList))
            {
                if (!eventRemoveScene.IsActive())
                    cardSystem.DealNewHand(baseHandSize, objectsList);
            }
        }
        return;
    }

    if (shopSystem.IsActive())
    {
        if (type == 3)
        {
            shopSystem.HandleHover(mousePos.x, mousePos.y);
        }
        else if (type == 0)
        {
            shopSystem.HandleMouseClick(mousePos, cardSystem, playerData, objectsList);
            UpdateHPBar();
            if (!shopSystem.IsActive() && inShopOnlyLevel)
                AdvanceToNextRound();
        }
        else if (type == 4)
        {
            Card* shopCard = shopSystem.PeekCardAt(mousePos.x, mousePos.y);
            if (shopCard)
            {
                if (cardInspect.IsInspecting(shopCard))
                    cardInspect.Hide(objectsList);
                else
                    cardInspect.Show(shopCard, cardSystem, shopSystem.GetShopLoader(), objectsList);
            }
            else
            {
                cardInspect.Hide(objectsList);
            }
        }
        return;
    }

    if (cardRewardSystem.IsActive())
    {
        if (type == 3)
        {
            cardRewardSystem.HandleHover(mousePos.x, mousePos.y);
        }
        else if (type == 4)
        {
            Card* rewardCard = cardRewardSystem.PeekCardAt(mousePos.x, mousePos.y);
            if (rewardCard)
            {
                if (cardInspect.IsInspecting(rewardCard))
                    cardInspect.Hide(objectsList);
                else
                    cardInspect.Show(rewardCard, cardSystem, cardRewardSystem.GetRewardLoader(), objectsList);
            }
            else
            {
                cardInspect.Hide(objectsList);
            }
        }
        else if (type == 0)
        {
            cardInspect.Hide(objectsList);
            if (cardRewardSystem.HandleMouseClick(mousePos, cardSystem, objectsList))
            {
                rewardBoxScene.ClaimCard();
                if (rewardBoxScene.IsDone())
                    AdvanceToNextRound();
            }
        }
        return;
    }

    if (rewardBoxScene.IsActive())
    {
        if (type == 3)
        {
            rewardBoxScene.HandleHover(mousePos.x, mousePos.y);
        }
        else if (type == 0)
        {
            int clicked = rewardBoxScene.HandleClick(mousePos.x, mousePos.y, objectsList);
            if (clicked == 1 || clicked == 3)
            {
                // Coin row kept or skipped — grant coins now
                playerData.AddCoins(pendingCoinsEarned);
                std::cout << "[" << levelManager.GetLevelText() << "] Earned "
                          << pendingCoinsEarned << " coins." << std::endl;
            }
            if (clicked == 2)
            {
                cardRewardSystem.Open(objectsList);
                if (!cardRewardSystem.IsActive())
                {
                    // Card pool empty, auto-claim
                    rewardBoxScene.ClaimCard();
                }
            }
            if (rewardBoxScene.IsDone())
                AdvanceToNextRound();
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

    // Info button (existing UI) doubles as view-map shortcut
    auto viewMapIconHit = [&]() {
        return viewMapIcon
            && realX >= 805.0f - 40.0f && realX <= 805.0f + 40.0f
            && realY >= 500.0f - 40.0f && realY <= 500.0f + 40.0f;
    };

    if (viewMapHintText)
    {
        if (viewMapIconHit())
            viewMapHintText->SetPosition(glm::vec3(800.0f, 440.0f, 20.0f));
        else
            viewMapHintText->SetPosition(Button::HiddenHintPosition());
    }

    if (type == 0 && viewMapIconHit())
    {
        if (!mapSceneActive)
        {
            cardInspect.Hide(objectsList);
            if (deckViewer.IsActive()) deckViewer.Hide(objectsList);
            mapScene.OpenViewOnly(levelManager.GetLevel(), objectsList);
            mapSceneActive = true;
        }
        return;
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
        if (type == 3)
            deckViewer.HandleHover(mousePos);
        else if (type == 0)
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
        // Pause button (world pos 900,500, size 80x80)
        if (realX >= 860.0f && realX <= 940.0f && realY >= 460.0f && realY <= 540.0f)
        {
            cardSystem.UpdateHover(mousePos, true, objectsList);
            pauseMenu.Show(objectsList);
            pauseMenuActive = true;
            return;
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
            cardSystem.DiscardHandAndDraw(baseHandSize, objectsList);
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
                        playerAttacking = true;
                        attackTimer = 0;

                        const std::string& pid = result.pendingAttacks[0].patternId;
                        int patNum = pid.size() > 1 ? std::stoi(pid.substr(1)) : 0;
                        int attackType = 0; // 0 = normal, 1 = aoe, 2 = range
                        if ((patNum >= 5 && patNum <= 13) || (patNum >= 18 && patNum <= 22))
                            attackType = 1;
                        else if (patNum >= 14 && patNum <= 17)
                            attackType = 2;

                        // Choose attack type
                        if (attackType == 0)
                        {
                            playerData.SetPlayerAttack(ConvertDir(playerDir));
                        }
                        else if (attackType == 1)
                        {
                            playerData.SetPlayerAttackAoe((int)playerDir);
                        }
                        else if (attackType ==2)
                        {
                            playerData.SetPlayerAttackRange((int)playerDir);
                        }

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

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;

        bool hit = false;

        for (const auto& item : cells)
        {
            int row = item.first.x;
            int col = item.first.y;

            if (e->OccupiesTile(row, col))
            {
                hit = true;
                break;
            }
        }

        if (hit)
        {
            // While batteries are alive the player cannot damage the Boss
            if (dynamic_cast<Boss*>(e) && AnyBatteryAlive())
            {
                std::cout << "[Battery Shield] Boss is protected — damage blocked!\n";
                continue;
            }

            e->getDamage(1);
            cout << "HIT!!! Enemy HP: " << e->getHealth() << endl;
        }
    }

    cout << endl;
}
void Level::ApplyEnemyAttack(Enemy* e)
{
    if (!e || e->getIsDead() || e->getHealth() <= 0)
        return;

    if (!playersprite)
        return;

    e->PlayAttackAnimation(playersprite->GetPosition());
    e->showAttackText();

    // EliteEnemy1: full-row attack; heals friendly Elite1s it hits
    if (EliteEnemy1* elite1 = dynamic_cast<EliteEnemy1*>(e))
    {
        SpawnElite1Projectile(elite1);

        auto rowTiles = elite1->GetRowAttackTiles(GridStartRow, GridEndRow);
        for (auto& tile : rowTiles)
        {
            int r = tile.first, c = tile.second;
            if (nowRow == r && nowCol == c)
                PlayerTakeDamage(elite1->getAttackDamage());

            for (auto* other : enemies)
            {
                if (!other || other == elite1 || other->getIsDead() || other->getHealth() <= 0) continue;
                if (other->getNowRow() == r && other->getNowCol() == c)
                {
                    if (elite1->ShouldHealInstead(other))
                    {
                        int healed = std::min(other->getHealth() + elite1->GetHealAmount(),
                                              other->getMaxHealth());
                        other->setHealth(healed);
                    }
                }
            }
        }
        return;
    }

    // EliteEnemy2: cycling pattern or fallback cross, then advance phase
    if (EliteEnemy2* elite2 = dynamic_cast<EliteEnemy2*>(e))
    {
        int lockedRow = elite2->getLockedPlayerRow();
        int lockedCol = elite2->getLockedPlayerCol();

        std::vector<std::pair<int, int>> hitTiles;
        if (elite2->IsPlayerInPatternRange(lockedRow, lockedCol))
        {
            hitTiles = elite2->GetCurrentPatternTiles();
            elite2->AdvancePattern(); // only advance when in-range pattern fires
        }
        else
        {
            hitTiles = elite2->GetCrossAttackTiles(lockedRow, lockedCol);
            SpawnElite2Projectile(lockedRow, lockedCol);
            // pattern phase stays unchanged — waits for player to step into range
        }

        for (auto& tile : hitTiles)
        {
            if (nowRow == tile.first && nowCol == tile.second)
                PlayerTakeDamage(elite2->getAttackDamage());
        }
        return;
    }

    // Regular enemies (including Boss)
    int centerRow = e->getNowRow();
    int centerCol = e->getNowCol();
    if (dynamic_cast<Boss*>(e))
        centerCol += 2;

    auto attacks = e->getLockedAttackPattern().applyTo(centerRow, centerCol);

    for (auto& cell : attacks)
    {
        int x = cell.first.x;
        int y = cell.first.y;
        if (nowRow == x && nowCol == y)
            PlayerTakeDamage(e->getAttackDamage());
    }

    // ---- Boss attack visual effects ----
    if (Boss* b = dynamic_cast<Boss*>(e))
    {
        int choice = b->getAttackPatternChoice();

        // Collect in-bounds hit tiles
        std::vector<std::pair<int,int>> hitTiles;
        for (auto& cell : attacks)
        {
            int r = cell.first.x, c = cell.first.y;
            if (r >= GridStartRow && r < GridEndRow && c >= GridStartCol && c < GridEndCol)
                hitTiles.push_back({ r, c });
        }

        if (choice == 1 || choice == 2 || choice == 3)
        {
            SpawnBossAttack1(hitTiles);
        }
        else if (choice == 6 || choice == 7)
        {
            SpawnBossAttack2Falling(hitTiles);
        }
        else if (choice == 8)
        {
            bool enhanced = (b->getHealth() * 2 < b->getMaxHealth());
            SpawnBossAttackCross(b->getLockedPlayerRow(), b->getLockedPlayerCol(), enhanced);
        }
        // choice 9 (summon): no tile-attack visual
    }
}

bool Level::EnemyCanAttackPlayer(Enemy* e)
{
    if (!e) return false;

    // Batteries never attack the player
    if (dynamic_cast<Battery*>(e)) return false;

    if (e == bossEnemy)
        return true;

    // Elite1 attacks when its column aligns with the player's column
    if (dynamic_cast<EliteEnemy1*>(e))
        return e->getNowCol() == nowCol;

    // Elite2 always attacks from any position (pattern caster)
    if (dynamic_cast<EliteEnemy2*>(e))
        return true;

    int er = e->getNowRow();
    int ec = e->getNowCol();
    return (abs(er - nowRow) + abs(ec - nowCol) == 1);
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
        UpdateBossPlayerPos();
        if (enemies.empty())
        {
            turnState = TurnState::PLAYER_TURN;
            return;
        }

        if (currentEnemyIndex >= enemies.size())
        {
            currentEnemyIndex = 0;
            turnState = TurnState::END_TURN;
            return;
        }

        Enemy* e = enemies[currentEnemyIndex];
        if (e == bossEnemy) UpdateBossHPBar();
        if (!e || e->getIsDead() || e->getHealth() <= 0)
        {
            currentEnemyIndex++;
            return;
        }

        if (!enemyActing)
        {
            enemyActing = true;
            e->stepsRemaining = e->getMoveRange();

            if (e->isDelayed())
            {
                e->decrementDelay();
                enemyActing = false;
                currentEnemyIndex++;
                return;
            }
        }

        if (e->getIsMoving())
        {
            return;
        }

        if (e->isPreparingAttack())
        {
            if (e->getCountDownR() <= 0)
            {
                ApplyEnemyAttack(e);
                if (Boss* b = dynamic_cast<Boss*>(e))
                {
                    int summons = b->TryGetSummon();
                    for (int i = 0; i < summons; i++)
                        SpawnBossSummon();
                }
                highlightManager.HideEnemyAttack(e->highlightIndex);

                e->setPreparingAttack(false);

                enemyActing = false;
                currentEnemyIndex++;
                e->addDamage();
                return;
            }
            enemyActing = false;
            currentEnemyIndex++;
            return;
        }

        if (EnemyCanAttackPlayer(e) && !e->isPreparingAttack())
        {
            e->setPreparingAttack(true);
            e->setCountDownR();
            e->LockAttackPattern(nowRow, nowCol);

            PreviewAllEnemyAttacks();

            enemyActing = false;
            currentEnemyIndex++;
            return;
        }

        if (e->stepsRemaining > 0)
        {
            int newR, newC;
            if (e->TryMoveTowardPlayer(
                nowRow, nowCol,
                GridStartRow, GridEndRow,
                GridStartCol, GridEndCol,
                enemies,
                [this](int r, int c)
                {
                    return IsWalkable(r, c);
                },
                newR, newC))
            {
                e->setNowPosition(newR, newC);

                glm::vec3 world = GridToWorld(newR, newC);
                e->StartMove(world);

                e->stepsRemaining--;
                return;
            }
            else
            {
                e->stepsRemaining = 0;
            }
        }

        enemyActing = false;
        currentEnemyIndex++;

        break;
    }

    case TurnState::END_TURN:
    {
        highlightManager.HideEnemyAttack(enemyHighlightIndex);

        for (auto* e : enemies)
        {
            if (!e || e->getIsDead() || e->getHealth() <= 0) continue;

            e->decrementWeaken();

            if (e->isPreparingAttack())
            {
                e->decreaseCountDownR();
            }
        }

        PreviewAllEnemyAttacks();

        EndTurn();
        break;
    }
    }
}

void Level::ApplyEventEffect(EventScene::EffectType effect)
{
    switch (effect)
    {
    case EventScene::EffectType::EXTRA_DRAW:
        baseHandSize++;
        break;

    case EventScene::EffectType::GOLD_BONUS:
        goldBonusActive = true;
        break;

    case EventScene::EffectType::START_BARRIER:
        startCombatBarrier = 1;
        break;

    case EventScene::EffectType::START_OVERCLOCK:
        startCombatOverclock = 3;
        break;

    case EventScene::EffectType::MAX_HP:
        playerData.setMaxHp(playerData.getMaxHp() + 10);
        playerData.HealHp(10);
        UpdateHPBar();
        break;

    case EventScene::EffectType::CURRENCY:
        playerData.AddCoins(300);
        break;

    case EventScene::EffectType::REMOVE_CARDS:
        eventRemoveScene.Open(cardSystem, objectsList, 2);
        break;
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
    AttackPattern oriented = CardActionExecutor::OrientPattern(*pat, dz);

    auto cells = oriented.applyTo(nowRow, nowCol);

    highlightManager.ShowAttackPattern(
        cells,
        GridStartRow, GridEndRow,
        GridStartCol, GridEndCol,
        walkable,
        [this](int r, int c) { return GridToWorld(r, c); }
    );
}

void Level::PreviewMovePath(int steps, int dir)
{
    std::vector<std::pair<int, int>> enemyPositions;

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;

        for (auto& tile : e->GetOccupiedTiles())
            enemyPositions.push_back(tile);

        // Treat every tile in a living Elite1's row as blocked for the preview
        if (e->getHealth() > 0 && dynamic_cast<EliteEnemy1*>(e))
        {
            int guardedRow = e->getNowRow();
            for (int c = GridStartCol; c < GridEndCol; c++)
                enemyPositions.push_back({ guardedRow, c });
        }
    }

    highlightManager.ShowMovePreview(
        nowRow,
        nowCol,
        steps,
        dir,
        GridStartRow, GridEndRow,
        GridStartCol, GridEndCol,
        walkable,
        [this](int r, int c)
        {
            return GridToWorld(r, c);
        },
        enemyPositions
    );
}

void Level::PreviewAllEnemyAttacks()
{
    highlightManager.HideEnemyAttack(enemyHighlightIndex);

    // Helper: convert {row,col} pairs to the Cell format ShowEnemyAttack expects
    auto toHighlightCells = [](const std::vector<std::pair<int, int>>& tiles)
    {
        std::vector<std::pair<IVec2, int>> cells;
        cells.reserve(tiles.size());
        for (auto& tile : tiles)
            cells.push_back({ IVec2(tile.first, tile.second), 1 });
        return cells;
    };

    for (auto* e : enemies)
    {
        if (!e || e->getIsDead() || e->getHealth() <= 0) continue;
        if (!e->isPreparingAttack()) continue;

        std::vector<std::pair<IVec2, int>> cells;

        if (EliteEnemy1* elite1 = dynamic_cast<EliteEnemy1*>(e))
        {
            cells = toHighlightCells(elite1->GetRowAttackTiles(GridStartRow, GridEndRow));
        }
        else if (EliteEnemy2* elite2 = dynamic_cast<EliteEnemy2*>(e))
        {
            int lockedRow = elite2->getLockedPlayerRow();
            int lockedCol = elite2->getLockedPlayerCol();

            std::vector<std::pair<int, int>> tiles;
            if (elite2->IsPlayerInPatternRange(lockedRow, lockedCol))
                tiles = elite2->GetCurrentPatternTiles();
            else
                tiles = elite2->GetCrossAttackTiles(lockedRow, lockedCol);
            cells = toHighlightCells(tiles);
        }
        else
        {
            int centerRow = e->getNowRow();
            int centerCol = e->getNowCol();
            if (dynamic_cast<Boss*>(e))
                centerCol += 2;
            cells = e->getLockedAttackPattern().applyTo(centerRow, centerCol);
        }

        highlightManager.ShowEnemyAttack(
            cells,
            GridStartRow, GridEndRow,
            GridStartCol, GridEndCol,
            walkable,
            [this](int r, int c)
            {
                return GridToWorld(r, c);
            },
            enemyHighlightIndex
        );
    }
}
void Level::UpdateHPBar()
{
    if (!hpBar || !hpMask) return;

    float fullWidth = 300.0f;

    int hp    = std::max(0, std::min(playerData.getHp(),    playerData.getMaxHp()));
    int maxHp = playerData.getMaxHp();

    float percent = (maxHp > 0) ? (float)hp / (float)maxHp : 0.0f;

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

    playerData.setHp(playerData.getHp() - damage);
    UpdateHPBar();

    std::cout << "[Player] Took " << damage
        << " damage. HP: " << playerData.getHp() << "/" << playerData.getMaxHp() << std::endl;

    if (playerData.getHp() <= 0)
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

void Level::SpawnElite1Projectile(EliteEnemy1* elite1)
{
    if (!elite1 || !elite1->getObject()) return;

    // Determine travel direction: shoot toward the opposite edge
    int midRow  = (GridStartRow + GridEndRow) / 2;
    bool goRight = (elite1->getNowRow() < midRow);
    int endRow   = goRight ? (GridEndRow - 1) : GridStartRow;

    glm::vec3 start = GridToWorld(elite1->getNowRow(), elite1->getNowCol());
    glm::vec3 end   = GridToWorld(endRow,              elite1->getNowCol());
    start.z = 6.0f;
    end.z   = 6.0f;

    Elite1Projectile proj;
    proj.sprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy1Shoot.png", 1, 7);
    // Sprite faces left by default; flip when going right (same convention as EliteEnemy1)
    float sz = 120.0f; 
    proj.sprite->SetSize(goRight ? -sz : sz, -sz);
    proj.sprite->SetAnimationLoop(0, 0, 4, 80);
    proj.sprite->SetPosition(start);
    proj.startPos = start;
    proj.endPos   = end;
    proj.timer    = 0.0f;
    proj.duration = 1200.0f;
    proj.done     = false;

    objectsList.push_back(proj.sprite);
    elite1Projectiles.push_back(proj);
}

void Level::SpawnElite2Projectile(int centerRow, int centerCol)
{
    glm::vec3 target = GridToWorld(centerRow, centerCol);

    // Drop from well above the top of the visible grid
    glm::vec3 start = glm::vec3(target.x, 650.0f, 6.0f);
    glm::vec3 end   = glm::vec3(target.x, target.y, 6.0f);

    Elite2Projectile proj;
    proj.sprite = new SpriteObject("../Resource/Texture/Enemy/EliteEnemy2Shoot.png", 2, 6);
    float sz = 150.0f;
    proj.sprite->SetSize(sz, -sz);
    proj.sprite->SetRotate(90.0f); // 90° CCW so the sprite faces downward
    proj.sprite->SetAnimationLoop(0, 0, 4, 80);
    proj.sprite->SetPosition(start);
    proj.startPos = start;
    proj.endPos   = end;
    proj.timer    = 0.0f;
    proj.duration = 1200.0f; // ms to reach the target
    proj.done     = false;

    objectsList.push_back(proj.sprite);
    elite2Projectiles.push_back(proj);
}

void Level::LoadEnemyData()
{
    EnemyDatabase::LoadFromFile(PATH_ENEMY_DATA);
    EnemyLoadPattern::LoadFromFile(PATH_ENEMY_PATTERN);
}

void Level::InitBossLevel()
{
    boss = true;

    // Hide grid tiles that fall inside the boss's occupied area (rows 2-6, cols 0-1).
    // Tiles are stored row-major so tile(row,col) = gridTiles[row * GRID_COLS + col].
    for (int row = 2; row <= 6; row++)
    {
        for (int col = 0; col <= 1; col++)
        {
            int idx = row * GRID_COLS + col;
            if (idx < (int)gridTiles.size() && gridTiles[idx])
                gridTiles[idx]->SetSize(0.0f, 0.0f);
        }
    }

    // Remove stale HP bar objects if they exist (e.g. re-entering boss level)
    auto removeObj = [&](ImageObject*& ptr)
    {
        if (!ptr) return;
        objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), ptr), objectsList.end());
        delete ptr;
        ptr = nullptr;
    };
    removeObj(bossHpBg);
    removeObj(bossHpBar);
    removeObj(bossHpMask);

    // Spawn boss
    bossEnemy = new Boss();
    bossEnemy->setNowPosition(4, 0);
    bossEnemy->SetWorldPosition(GridToWorld(4, 0));

    enemies.push_back(bossEnemy);
    objectsList.push_back(bossEnemy->getObject());
    objectsList.push_back(bossEnemy->getHPText());
    objectsList.push_back(bossEnemy->getCorruptText());
    objectsList.push_back(bossEnemy->getDebuffText());

    // Boss HP bar UI
    {
        ImageObject* bg = new ImageObject();
        bg->SetSize(600.0f, -100.0f);
        bg->SetPosition(glm::vec3(0.0f, 490.0f, 0.0f));
        bg->SetTexture("../Resource/Texture/UI/Boss_HPbar_Blank.PNG");
        objectsList.push_back(bg);
        bossHpBg = bg;
    }
    {
        bossHpBar = new ImageObject();
        bossHpBar->SetSize(600.0f, -100.0f);
        bossHpBar->SetPosition(glm::vec3(0.0f, 490.0f, 1.0f));
        bossHpBar->SetTexture("../Resource/Texture/UI/Boss_HPbar.PNG");
        objectsList.push_back(bossHpBar);
    }
    {
        bossHpMask = new ImageObject();
        bossHpMask->SetSize(0.0f, -50.0f);
        bossHpMask->SetPosition(glm::vec3(0.0f, 490.0f, 5.0f));
        bossHpMask->SetTexture("../Resource/Texture/UI/HPbarmask.png");
        objectsList.push_back(bossHpMask);
    }
}

void Level::RestoreEnemiesFromSave(const SaveData& sd)
{
    for (const auto& esd : sd.enemies)
    {
        Enemy* e = nullptr;

        if (esd.typeIndex == 11)
        {
            // Boss is already spawned in LevelInit; just restore its health.
            if (bossEnemy)
                bossEnemy->setHealth(esd.health);
            continue;
        }
        else if (esd.typeIndex == 10)
            e = new EliteEnemy2();
        else if (esd.typeIndex == 9)
            e = new EliteEnemy1();
        else
            e = new Enemy(static_cast<Enemy::EnemyType>(esd.typeIndex));

        e->setNowPosition(esd.row, esd.col);
        e->SetWorldPosition(GridToWorld(esd.row, esd.col));
        e->setHealth(esd.health);
        if (esd.delayTurns       > 0) e->addDelay(esd.delayTurns);
        if (esd.corruptionStacks > 0) e->addCorruption(esd.corruptionStacks);
        if (esd.weakenTurns      > 0) e->addWeaken(esd.weakenTurns);

        enemies.push_back(e);
        objectsList.push_back(e->getObject());
        objectsList.push_back(e->getHPText());
        objectsList.push_back(e->getCorruptText());
        objectsList.push_back(e->getDebuffText());
        objectsList.push_back(e->getCountdownIcon());
        objectsList.push_back(e->getCountdownText());

        std::cout << "[Load] Restored enemy type=" << esd.typeIndex
                  << " at (" << esd.row << "," << esd.col
                  << ") hp=" << esd.health << "\n";
    }
}

void Level::SpawnEnemiesForLevel()
{
    LevelConfig cfg = levelManager.GetCurrentConfig();

    // Boss level — full setup (spawn + HP bar).
    if (cfg.type == LevelConfig::Type::Boss)
    {
        InitBossLevel();
        return;
    }

    // ---- Helpers ------------------------------------------------------------

    // Place an elite on a specific row, random valid column.
    auto SpawnEliteAtRow = [&](Enemy* e, int fixedRow, int index)
    {
        std::vector<int> validCols;
        for (int c = GridStartCol; c < GridEndCol; c++)
        {
            if (!walkable[fixedRow][c]) continue;
            if (abs(fixedRow - nowRow) <= 1 && abs(c - nowCol) <= 1) continue;
            bool occupied = false;
            for (auto* ex : enemies)
            {
                if (!ex || ex->getIsDead()) continue;
                if (ex->OccupiesTile(fixedRow, c)) { occupied = true; break; }
            }
            if (occupied) continue;
            validCols.push_back(c);
        }
        if (validCols.empty())
        {
            std::cout << "[Spawn] No valid col for elite " << index
                      << " at row " << fixedRow << "\n";
            delete e;
            return;
        }
        int spawnCol = validCols[rand() % (int)validCols.size()];
        e->setNowPosition(fixedRow, spawnCol);
        e->SetWorldPosition(GridToWorld(fixedRow, spawnCol));
        enemies.push_back(e);
        objectsList.push_back(e->getObject());
        objectsList.push_back(e->getHPText());
        objectsList.push_back(e->getCorruptText());
        objectsList.push_back(e->getDebuffText());
        objectsList.push_back(e->getCountdownIcon());
        objectsList.push_back(e->getCountdownText());
        std::cout << "[Spawn] Elite " << index << " at ("
                  << fixedRow << ", " << spawnCol << ")\n";
    };

    // Place Elite2 at its fixed mid-grid position.
    auto SpawnElite2Fixed = [&](int index)
    {
        const int fixedRow = 6;
        const int fixedCol = 2;
        auto* e = new EliteEnemy2();
        e->setNowPosition(fixedRow, fixedCol);
        e->SetWorldPosition(GridToWorld(fixedRow, fixedCol));
        enemies.push_back(e);
        objectsList.push_back(e->getObject());
        objectsList.push_back(e->getHPText());
        objectsList.push_back(e->getCorruptText());
        objectsList.push_back(e->getDebuffText());
        objectsList.push_back(e->getCountdownIcon());
        objectsList.push_back(e->getCountdownText());
        std::cout << "[Spawn] Elite2 " << index << " at ("
                  << fixedRow << ", " << fixedCol << ") [fixed]\n";
    };

    // Place one normal enemy at a random unoccupied border tile.
    auto SpawnNormalEnemy = [&](Enemy::EnemyType type)
    {
        std::vector<std::pair<int, int>> valid;
        for (int r = GridStartRow; r < GridEndRow; r++)
        {
            for (int c = GridStartCol; c < GridEndCol; c++)
            {
                bool isBorder = (r == GridStartRow || r == GridEndRow - 1 ||
                                 c == GridStartCol  || c == GridEndCol  - 1);
                if (!isBorder)                                              continue;
                if (!walkable[r][c])                                        continue;
                if (abs(r - nowRow) <= 1 && abs(c - nowCol) <= 1)          continue;
                bool occupied = false;
                for (auto* ex : enemies)
                {
                    if (!ex || ex->getIsDead()) continue;
                    if (ex->OccupiesTile(r, c)) { occupied = true; break; }
                }
                if (occupied) continue;
                valid.push_back({ r, c });
            }
        }
        if (valid.empty())
        {
            std::cout << "[Spawn] No border tile available for normal enemy\n";
            return;
        }
        int pick = rand() % (int)valid.size();
        int spawnRow = valid[pick].first;
        int spawnCol = valid[pick].second;
        Enemy* e = new Enemy(type);
        e->setNowPosition(spawnRow, spawnCol);
        e->SetWorldPosition(GridToWorld(spawnRow, spawnCol));
        enemies.push_back(e);
        objectsList.push_back(e->getObject());
        objectsList.push_back(e->getHPText());
        objectsList.push_back(e->getCorruptText());
        objectsList.push_back(e->getDebuffText());
        objectsList.push_back(e->getCountdownIcon());
        objectsList.push_back(e->getCountdownText());
        std::cout << "[Spawn] Normal enemy at (" << spawnRow << ", " << spawnCol << ")\n";
    };

    // ---- Elite levels -------------------------------------------------------

    if (cfg.type == LevelConfig::Type::Elite1)
    {
        SpawnEliteAtRow(new EliteEnemy1(), GridStartRow,   1);
        SpawnEliteAtRow(new EliteEnemy1(), GridEndRow - 1, 2);
        return;
    }
    if (cfg.type == LevelConfig::Type::Elite2)
    {
        SpawnElite2Fixed(1);
        return;
    }
    if (cfg.type == LevelConfig::Type::EliteRandom)
    {
        if (elite1)
        {
            SpawnEliteAtRow(new EliteEnemy1(), GridStartRow,   1);
            SpawnEliteAtRow(new EliteEnemy1(), GridEndRow - 1, 2);
        }
        else
        {
            SpawnElite2Fixed(1);
        }
        return;
    }

    // ---- Normal combat levels -----------------------------------------------

    for (const auto& group : cfg.groups)
    {
        const auto& pool = group.pool;
        for (int i = 0; i < group.count; i++)
        {
            Enemy::EnemyType type = pool[rand() % (int)pool.size()];
            SpawnNormalEnemy(type);
        }
    }
}

void Level::AdvanceToNextRound()
{
    int fromLevel = levelManager.GetLevel();
    levelManager.Advance();
    int toLevel = levelManager.GetLevel();

    // Close overlays that should not appear behind the map screen
    rewardBoxScene.Close(objectsList);
    cardInspect.Hide(objectsList);
    if (deckViewer.IsActive()) deckViewer.Hide(objectsList);

    // Show the map scene with fade-in, player walk, fade-out
    mapScene.Open(fromLevel, toLevel, objectsList);
    mapSceneActive = true;
}

void Level::ResetForNextCombat()
{
    // Update level indicator UI
    if (winText)   winText->SetPosition(glm::vec3(0.0f, 10000.0f, 10.0f));
    if (levelText)
    {
        SDL_Color color = { 255, 230, 100, 255 };
        levelText->LoadText(levelManager.GetLevelText(), color, 30);
    }

    // Show level name banner for 2 seconds
    if (levelNameBanner)
    {
        SDL_Color color = { 255, 230, 100, 255 };
        levelNameBanner->LoadText(levelManager.GetLevelText(), color, 80);
        levelNameBanner->SetPosition(glm::vec3(0.0f, 480.0f, 11.0f));
        levelBannerTimer = 0.0f;
        levelBannerActive = true;
    }

    winDelayActive = false;
    winDelay = 0.0f;

    // Reset combat flags
    isGameOver             = false;
    rewardPickedAfterWin   = false;
    shopOpenedAfterWin     = false;
    inShopOnlyLevel        = false;
    playerDead             = false;
    anyEnemyDied           = false;
    enemyActing            = false;
    currentEnemyIndex      = 0;
    enemyPreparingAttack   = false;
    enemyHighlightIndex    = 0;

    // Reset turn state
    turnState       = TurnState::PLAYER_TURN;
    turnCount       = 0;
    lagTurns        = 0;
    tempDiscardDone = false;

    // Reset player movement / animation
    playerMoving         = false;
    playerAttacking      = false;
    playerMoveTimer      = 0.0f;
    attackTimer          = 0.0f;
    pendingAttack        = false;
    pendingMoveSteps     = 0;
    pendingMoveZone      = -1;
    pendingFastCard      = false;
    playerPlayingOneShot = false;
    playerAnimTimer      = 0.0f;
    playerAnimDuration   = 0.0f;
    currentPatternIndex  = 0;
    currentRotation      = 0;

    // Reset boss/elite state before SetPlayerSpawnPosition so spawn uses correct flags
    bossActed = false;
    boss   = levelManager.IsBossLevel();
    elite1 = false;
    elite2 = false;
    {
        LevelConfig cfg = levelManager.GetCurrentConfig();
        if (cfg.type == LevelConfig::Type::Elite1) elite1 = true;
        else if (cfg.type == LevelConfig::Type::Elite2) elite2 = true;
        else if (cfg.type == LevelConfig::Type::EliteRandom)
        {
            if (rand() % 2 == 0) elite1 = true;
            else                  elite2 = true;
        }
    }

    // Reset walkable array for the new level, then apply boss constraints if needed
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            walkable[r][c] = true;
    if (boss)
        for (int col = 0; col < 2; col++)
            for (int row = 2; row < 7; row++)
                walkable[row][col] = false;

    // Move player back to start; HP and coins carry over
    SetPlayerSpawnPosition();
    playerDir   = PlayerDir::DOWN;
    playerState = PlayerState::IDLE;
    playerData.SetPosition(GridToWorld(nowRow, nowCol));
    UpdatePlayerAnimation();
    playerData.ResetShield();
    playerData.ResetJumpCharges();
    UpdateHPBar();

    // Auto-save when player first enters this level
    {
        SaveData sd;
        sd.playerRow            = nowRow;
        sd.playerCol            = nowCol;
        sd.playerHp             = playerData.getHp();
        sd.playerMaxHp          = playerData.getMaxHp();
        sd.playerCoins          = playerData.GetCoins();
        sd.playerBarrierCount   = playerData.GetBarrierCount();
        sd.playerJumpCharges    = playerData.GetJumpCharges();
        sd.currentLevel         = levelManager.GetLevel();
        sd.baseHandSize         = baseHandSize;
        sd.goldBonusActive      = goldBonusActive;
        sd.startCombatBarrier   = startCombatBarrier;
        sd.startCombatOverclock = startCombatOverclock;
        sd.eventSceneDone       = eventSceneDone;
        for (Card* c : cardSystem.GetAllCards())
            sd.cardNames.push_back(c->getName());
        SaveSystem::Save(sd);
    }

    // Hide remaining overlays
    highlightManager.HideAllPlayer();
    highlightManager.HideAllEnemy();

    // Remove old enemies
    for (auto* e : enemies)
    {
        if (!e) continue;
        auto removeObj = [&](DrawableObject* obj) {
            if (obj) objectsList.erase(std::remove(objectsList.begin(), objectsList.end(), obj), objectsList.end());
        };
        removeObj(e->getObject());
        removeObj(e->getHPText());
        removeObj(e->getCorruptText());
        removeObj(e->getDebuffText());
        removeObj(e->getCountdownIcon());
        removeObj(e->getCountdownText());
        delete e;
    }
    enemies.clear();
    bossEnemy = nullptr; // pointer is dangling after delete above

    // Expire lingering damage popups
    for (auto& p : damagePopups)
        if (!p.expired && p.text) p.text->SetSize(0.0f, 0.0f);
    damagePopups.clear();

    // Hide lingering Elite1 projectiles
    for (auto& proj : elite1Projectiles)
        if (!proj.done && proj.sprite) proj.sprite->SetSize(0.0f, 0.0f);
    elite1Projectiles.clear();

    // Hide lingering Elite2 projectiles
    for (auto& proj : elite2Projectiles)
        if (!proj.done && proj.sprite) proj.sprite->SetSize(0.0f, 0.0f);
    elite2Projectiles.clear();

    // Hide lingering Boss attack effects
    for (auto& fx : bossAttack1Effects)
        if (!fx.done && fx.sprite) fx.sprite->SetSize(0.0f, 0.0f);
    bossAttack1Effects.clear();

    for (auto& proj : bossAttack2Projectiles)
        if (!proj.done && proj.sprite) proj.sprite->SetSize(0.0f, 0.0f);
    bossAttack2Projectiles.clear();

    for (auto& portal : bossSummonPortals)
        if (!portal.done && portal.sprite) portal.sprite->SetSize(0.0f, 0.0f);
    bossSummonPortals.clear();

    std::cout << "=== " << levelManager.GetLevelText() << " ===" << std::endl;

    // Shop-only levels (4, 9, 14, 19): open shop, skip combat setup
    if (levelManager.IsShopOnlyLevel())
    {
        inShopOnlyLevel = true;
        turnState = TurnState::GAME_OVER;
        shopSystem.Open(objectsList, playerData);
        shopOpenedAfterWin = true;
        return;
    }

    // Regular combat level: spawn enemies and reset card system
    LoadEnemyData();
    SpawnEnemiesForLevel();

    std::string err;
    cardSystem.Clear(objectsList);
    cardSystem.LoadData(PATH_PATTERN, PATH_CARDS_STARTER, PATH_CARD_DESC, &err);
    cardRewardSystem.ApplyOwnedRewards(cardSystem);
    shopSystem.ApplyRemovals(cardSystem);
    cardSystem.InitUI(objectsList);
    cardSystem.ShuffleDeck();
    cardSystem.DealNewHand(baseHandSize, objectsList);

    if (startCombatBarrier   > 0) playerData.AddBarrier(startCombatBarrier);
    if (startCombatOverclock > 0) cardSystem.ApplyOverclock(startCombatOverclock);
}

bool Level::IsWalkable(int row, int col) const
{
    if (row < 0 || row >= GRID_ROWS ||
        col < 0 || col >= GRID_COLS)
    {
        return false;
    }

    return walkable[row][col];
}
void Level::SpawnBossSummon()
{
    std::vector<std::pair<int, int>> validTiles;

    for (int r = GridStartRow; r < GridEndRow; r++)
    {
        for (int c = GridStartCol; c < GridEndCol; c++)
        {
            if (!walkable[r][c]) continue;

            // Must not be within the 3x3 block surrounding the player
            if (abs(r - nowRow) <= 1 && abs(c - nowCol) <= 1) continue;

            // Must not be occupied by any enemy
            bool occupied = false;
            for (auto* e : enemies)
            {
                if (!e || e->getIsDead()) continue;
                if (e->OccupiesTile(r, c)) { occupied = true; break; }
            }
            if (occupied) continue;

            validTiles.push_back({ r, c });
        }
    }

    if (validTiles.empty())
    {
        std::cout << "[Boss] No valid tile for summon!\n";
        return;
    }

    int idx = rand() % (int)validTiles.size();
    int spawnRow = validTiles[idx].first;
    int spawnCol = validTiles[idx].second;

    static const Enemy::EnemyType pool[] = {
        Enemy::EnemyType::A,
        Enemy::EnemyType::B,
        Enemy::EnemyType::C,
		Enemy::EnemyType::D,
		Enemy::EnemyType::E,
		Enemy::EnemyType::F,
		Enemy::EnemyType::G
    };
    Enemy::EnemyType type = pool[rand() % 7];

    // Spawn the portal — the enemy will appear on frame 4 (deferred in LevelUpdate)
    glm::vec3 portalPos = GridToWorld(spawnRow, spawnCol);
    portalPos.z = 6.0f;

    BossSummonPortal portal;
    portal.sprite    = new SpriteObject("../Resource/Texture/Boss/Summon.png", 1, 6);
    portal.sprite->SetSize(GridWide * 4.5f, -GridHigh * 4.5f); // 1.5 × 3 = 4.5× tile size
    portal.sprite->SetAnimationOnce(0, 0, 6, 300);   // 6 frames × 300 ms (half speed)
    portal.sprite->SetPosition(portalPos);
    portal.spawnRow    = spawnRow;
    portal.spawnCol    = spawnCol;
    portal.enemyType   = type;
    portal.timer       = 0.0f;
    portal.spawnDelay  = 3 * 300.0f;  // frame 4 starts after 3 frame-delays = 900 ms
    portal.enemySpawned = false;
    portal.done        = false;

    objectsList.push_back(portal.sprite);
    bossSummonPortals.push_back(portal);

    std::cout << "[Boss] Portal opening at (" << spawnRow << ", " << spawnCol
              << ") — enemy appears on frame 4\n";
}

// -----------------------------------------------------------------------
// Boss attack visuals
// -----------------------------------------------------------------------

// Grid 1 / 2 / 3 — stationary tile-flash effect (BossAttack1.PNG, 1 row × 7 cols)
void Level::SpawnBossAttack1(const std::vector<std::pair<int,int>>& tiles)
{
    for (auto& tile : tiles)
    {
        int r = tile.first, c = tile.second;

        // Skip tiles covered by the boss hitbox
        if (bossEnemy && bossEnemy->OccupiesTile(r, c)) continue;

        glm::vec3 pos = GridToWorld(r, c);
        pos.z = 7.0f;

        BossAttack1Effect fx;
        fx.sprite = new SpriteObject("../Resource/Texture/Boss/BossAttack1.PNG", 1, 7);
        fx.sprite->SetSize(GridWide * 1.5f, -GridHigh * 1.5f);
        fx.sprite->SetAnimationOnce(0, 0, 7, 100);     // 7 frames × 100 ms
        fx.sprite->SetPosition(pos);
        fx.done = false;

        objectsList.push_back(fx.sprite);
        bossAttack1Effects.push_back(fx);
    }
}

// Grid 6 / 7 — projectiles fall from the sky (BossAttack2.PNG, rotated CCW 90°)
void Level::SpawnBossAttack2Falling(const std::vector<std::pair<int,int>>& tiles)
{
    for (auto& tile : tiles)
    {
        int r = tile.first, c = tile.second;

        glm::vec3 target = GridToWorld(r, c);
        glm::vec3 start  = glm::vec3(target.x, 650.0f, 6.0f);
        glm::vec3 end    = glm::vec3(target.x, target.y, 6.0f);

        BossAttack2Projectile proj;
        proj.sprite = new SpriteObject("../Resource/Texture/Boss/BossAttack2.PNG", 1, 7);
        float sz = 135.0f;
        proj.sprite->SetSize(sz, -sz);
        proj.sprite->SetRotate(90.0f);              // CCW 90° — sprite faces downward
        proj.sprite->SetAnimationLoop(0, 0, 4, 80);
        proj.sprite->SetPosition(start);
        proj.startPos = start;
        proj.endPos   = end;
        proj.timer    = 0.0f;
        proj.duration = 700.0f;
        proj.done     = false;

        objectsList.push_back(proj.sprite);
        bossAttack2Projectiles.push_back(proj);
    }
}

// Cross attack (choice 8) — projectiles travel from player center to each grid edge.
// Default sprite faces LEFT; rotated per direction.  Enhanced mode (HP < 50%) is 3× bigger.
void Level::SpawnBossAttackCross(int centerRow, int centerCol, bool enhanced)
{
    float sz = enhanced ? 405.0f : 135.0f;  // enhanced = x3, normal = x1.5 base size

    glm::vec3 center = GridToWorld(centerRow, centerCol);
    center.z = 6.0f;

    // Helper — create one arm travelling from center to endWorld
    auto spawnArm = [&](glm::vec3 endWorld, float sizeX, float sizeY, float rotDeg)
    {
        endWorld.z = 6.0f;
        BossAttack2Projectile proj;
        proj.sprite = new SpriteObject("../Resource/Texture/Boss/BossAttack2.PNG", 1, 7);
        proj.sprite->SetSize(sizeX, sizeY);
        proj.sprite->SetRotate(rotDeg);
        proj.sprite->SetAnimationLoop(0, 0, 4, 80);
        proj.sprite->SetPosition(center);
        proj.startPos = center;
        proj.endPos   = endWorld;
        proj.timer    = 0.0f;
        proj.duration = 600.0f;
        proj.done     = false;
        objectsList.push_back(proj.sprite);
        bossAttack2Projectiles.push_back(proj);
    };

    // Row axis = x axis.  Left = smaller row (smaller x), Right = larger row (larger x).
    glm::vec3 leftEdge  = GridToWorld(GridStartRow, centerCol);
    glm::vec3 rightEdge = GridToWorld(GridEndRow - 1, centerCol);
    // Col axis = y axis.  Up = smaller col (larger y), Down = larger col (smaller y).
    glm::vec3 upEdge    = GridToWorld(centerRow, GridStartCol);
    glm::vec3 downEdge  = GridToWorld(centerRow, GridEndCol - 1);

    // Left arm  — sprite already faces left, keep x positive
    spawnArm(glm::vec3(leftEdge.x,  center.y, 6.0f),  sz, -sz,    0.0f);
    // Right arm — flip x so sprite faces right
    spawnArm(glm::vec3(rightEdge.x, center.y, 6.0f), -sz, -sz,    0.0f);
    // Up arm    — rotate CW 90° so sprite faces upward (y-up space)
    spawnArm(glm::vec3(center.x, upEdge.y,   6.0f),   sz, -sz,  -90.0f);
    // Down arm  — rotate CCW 90° so sprite faces downward
    spawnArm(glm::vec3(center.x, downEdge.y, 6.0f),   sz, -sz,   90.0f);
}

bool Level::AnyBatteryAlive() const
{
    for (auto* e : enemies)
    {
        if (!e || e->getIsDead()) continue;
        if (dynamic_cast<const Battery*>(e)) return true;
    }
    return false;
}

void Level::SpawnBatteries()
{
    // Spawn two batteries: left edge bottom (row 0, col 4) and right edge bottom (row 8, col 4).
    // "col 4" is the far column away from the player start (bottom of the visual grid).
    const std::pair<int,int> spawnPoints[] = { {0, 4}, {8, 4} };

    for (auto& sp : spawnPoints)
    {
        int r = sp.first;
        int c = sp.second;

        Battery* bat = new Battery();
        bat->setNowPosition(r, c);
        bat->SetWorldPosition(GridToWorld(r, c));

        enemies.push_back(bat);
        objectsList.push_back(bat->getObject());
        objectsList.push_back(bat->getHPText());
        objectsList.push_back(bat->getCorruptText());
        objectsList.push_back(bat->getDebuffText());
        objectsList.push_back(bat->getCountdownIcon());
        objectsList.push_back(bat->getCountdownText());

        std::cout << "[Boss] Battery spawned at (" << r << ", " << c << ")\n";
    }

    // Play the dedicated battery-summon animation on the boss
    if (bossEnemy)
        bossEnemy->PlayBatterySummonAnimation();

    // Boss continues to attack normally while batteries are alive.
    // Player attacks on the boss are blocked (ApplyAttackCells checks AnyBatteryAlive).
}

void Level::SetPlayerSpawnPosition()
{
    if (boss)
    {
        nowRow = 4;
        nowCol = 3;
        return;
    }

    if (elite1)
    {
        nowRow = 4;
        nowCol = 2;
        return;
    }

    if (elite2)
    {
        nowRow = 1;
        nowCol = 2;
        return;
    }

    std::vector<std::pair<int, int>> validTiles;

    for (int r = GridStartRow + 1; r < GridEndRow - 1; r++)
    {
        for (int c = GridStartCol + 1; c < GridEndCol - 1; c++)
        {
            if (walkable[r][c])
                validTiles.push_back({ r, c });
        }
    }

    if (!validTiles.empty())
    {
        int idx = rand() % (int)validTiles.size();
        nowRow = validTiles[idx].first;
        nowCol = validTiles[idx].second;
    }
}

void Level::UpdateBossPlayerPos()
{
    if (bossEnemy)
        bossEnemy->setPlayerPosition(nowRow, nowCol);
}
void Level::UpdateBossHPBar()
{
    if (!bossHpBar || !bossEnemy) return;

    float fullWidth = 600.0f;
    float barY = 490.0f;

    int hp = std::max(0, bossEnemy->getHealth());
    int maxHp = bossEnemy->getMaxHealth();
    float percent = (maxHp > 0) ? (float)hp / maxHp : 0.0f;

    bossHpBar->SetSize(fullWidth * percent, -100.0f);
    bossHpBar->SetPosition(glm::vec3(0.0f, barY, 1.0f));
}