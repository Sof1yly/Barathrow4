#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

// Bezier line
static inline glm::vec3 QuadraticBezier(
    const glm::vec3& P0,
    const glm::vec3& C,
    const glm::vec3& P1,
    float t)
{
    float u = 1.0f - t;
    return (u * u) * P0 + 2.0f * u * t * C + (t * t) * P1;
}

// ===========================
// Lifecycle
// ===========================

void Level::LevelLoad()
{
    auto* square = new SquareMeshVbo();
    square->LoadData();
    GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);

    auto* sprite = new SpriteMeshVbo();
    sprite->LoadData();
    GameEngine::GetInstance()->AddMesh(SpriteMeshVbo::MESH_NAME, sprite);

    cout << "Load Level" << endl;
}

void Level::LevelInit()
{
    // 1) Tile grid (your original)
    for (int i = GridStartRow; i < GridEndRow; ++i) {
        for (int j = GridStartCol; j < GridEndCol; ++j) {
            ImageObject* tile = new ImageObject();
            tile->SetTexture("../Resource/Texture/tile.png");
            tile->SetSize(GridWide, GridHigh);
            tile->SetPosition(glm::vec3(
                i * 101.0f - 404.0f,
                j * -105.0f + 352.0f,
                0.0f
            ));
            objectsList.push_back(tile);
        }
    }

    //Enemy
	enemy = new Enemy();
    enemy->setNowPosition(8, 0); //row=8,col=0

	ImageObject* enemyObj = new ImageObject();
	enemyObj->SetSize(100.0f, -100.0f);
    enemyObj->SetTexture("../Resource/Texture/doro.png");

	//glm::vec3 pos = GridToWorld(enemy->getNowRow(), enemy->getNowCol()); //  glm::vec3 pos = GridToWorld(8, 0);
    glm::vec3 pos = GridToWorld(8, 0);
	enemyObj->SetPosition(pos);

	objectsList.push_back(enemyObj); 
    enemy->setObject(enemyObj);


    // 3) Player grid marker
    {
        ImageObject* marker = new ImageObject();
        marker->SetSize(84.0f, -90.0f);
        marker->SetPosition(glm::vec3(-404.0f, 352.0f, 0.0f));
        marker->SetTexture("../Resource/Texture/player.png");
        objectsList.push_back(marker);
        testGrid = marker;
    }

    // 4) Demo objects
    {
        GameObject* obj = new GameObject();
        obj->SetColor(1.0f, 0.0f, 0.0f);
        obj->SetSize(200.0f, 200.0f);
        objectsList.push_back(obj);
        player = obj;
    }

    {
        GameObject* obj2 = new GameObject();
        obj2->SetColor(0.0f, 1.0f, 0.0f);
        obj2->SetSize(50.0f, 50.0f);
        obj2->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
        objectsList.push_back(obj2);
    }

    {
        GameObject* obj3 = new GameObject();
        obj3->SetColor(0.0f, 0.0f, 1.0f);
        obj3->SetSize(100.0f, 100.0f);
        obj3->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        objectsList.push_back(obj3);
        testMove = obj3;
    }

    testMoveTarget = testMove->GetPosition();
    testMoveMoving = false;


    //  Menu button + UI
    {
        ImageObject* img = new ImageObject();
        img->SetSize(50.0f, -50.0f);
        img->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
        img->SetTexture("../Resource/Texture/menu.png");
        objectsList.push_back(img);
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
    if (!dataLoader.loadPatternsFromFile("../Resource/GameData/Pattern.txt", &error)) {
        std::cerr << "Error loading attack patterns: " << error << std::endl;
    }

    // 2) Load actions + cards (with Pattern column)
    if (!dataLoader.loadFromFile("../Resource/GameData/CardAction.txt", &error)) {
        std::cerr << "Error loading card data: " << error << std::endl;
    }

    deck = dataLoader.getCards();
    ShuffleDeck();

    // Start the game with 5 cards in hand
    DealNewHand(5);


    // ------------------------
    // Re-Draw button (LEFT)
    reDrawButton = new ImageObject();
    reDrawButton->SetSize(200.0f, -260.0f);
    reDrawButton->SetPosition(glm::vec3(-800.0f, -220.0f, 10.0f));  // LEFT
    reDrawButton->SetTexture("../Resource/Texture/cards/reDeck.png");
    objectsList.push_back(reDrawButton);

    // ------------------------
    // View-Deck button (RIGHT)
    viewDeckButton = new ImageObject();
    viewDeckButton->SetSize(200.0f, -260.0f);
    viewDeckButton->SetPosition(glm::vec3(800.0f, -220.0f, 10.0f));  // RIGHT
    viewDeckButton->SetTexture("../Resource/Texture/cards/deck.png");
    objectsList.push_back(viewDeckButton);


    // Drop zones
    CreateDropZones(objectsList);

	//////////////////////////////////////test sprite *delete later
    SpriteObject* sprite = new SpriteObject("../Resource/Texture/TestSprite.png", 4, 7);
    sprite->SetSize(50.0f, -50.0f);
    sprite->SetPosition(glm::vec3(800.0f, 0.0f, 0.0f));
    sprite->SetAnimationLoop(0, 0, 27, 50);
    sprite->NextAnimation();
    objectsList.push_back(sprite);
	//////////////////////////////////////test sprite *delete later

    std::cout << "Init Level" << std::endl;
}

void Level::LevelUpdate()
{
    int deltaTime = GameEngine::GetInstance()->GetDeltaTime();

    UpdateTurn();

    for (auto* obj : objectsList)
        obj->Update((float)deltaTime);

    if (enemy && enemy->getHealth() <= 0)
    {
        ImageObject* obj = enemy->getObject();

        if (obj)
        {
            // remove from vector
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end())
                objectsList.erase(it);

            delete obj;
            enemy->setObject(nullptr);
        }

        delete enemy;
        enemy = nullptr;
    }


    // Smooth move testMove object
    if (testMoveMoving && testMove) {
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
    }
}

void Level::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void Level::LevelFree()
{
    for (auto* obj : objectsList)
        delete obj;
    objectsList.clear();
    bezierSegments.clear();

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
    switch (key)
    {
    case 'q': GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;    break;
    case 'r': GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART; break;
    case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL3;  break;
    default: break;
    }

    if (!testGrid) return;

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

	if (key == 'w' && nowCol != 0) {
		testGrid->Translate(glm::vec3(0, GridHigh+distanceBetweenGridY, 0));
		nowCol--;
	}
	else if (key == 's' && nowCol < GridEndCol-1) {
		testGrid->Translate(glm::vec3(0, -(GridHigh + distanceBetweenGridY), 0));
		nowCol++;
	}
	else if (key == 'a'&&nowRow!=0) {
		testGrid->Translate(glm::vec3(-(GridWide + distanceBetweenGridX), 0, 0));
		nowRow--;
	}
	else if (key == 'd'&&nowRow<GridEndRow-1) {
		testGrid->Translate(glm::vec3(GridWide+distanceBetweenGridX, 0, 0));
		nowRow++;
	}
}

void Level::HandleMouse(int type, int x, int y)
{
    int winW = GameEngine::GetInstance()->GetWindowWidth();
    int winH = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

    float realX = (x - winW / 2.0f) * (scaleW / winW);
    float realY = (winH / 2.0f - y) * (scaleH / winH);
    glm::vec3 mousePos(realX, realY, 0.0f);

    screenCenterY = 0.0f;

    // ----------------------------------------------------
    // Hover
    // ----------------------------------------------------
    if (type == 3) {
        hand.UpdateHover(mousePos, isDragging);
        return;
    }

    // Left Click
    if (type == 0)
    {
        // menu toggle
        if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470)
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

        // ------------------------------
        // RE-DRAW BUTTON (LEFT) - Left Click
        //  - discard all cards in hand to discard pile
        //  - if deck is empty, move all discard back to deck and shuffle
        //  - draw new hand (5 cards)
        // ------------------------------
        if (reDrawButton && IsPointInsideZone(mousePos, reDrawButton))
        {
            cout << "[UI] Re-Draw Button clicked" << endl;
            
            // 1) Collect all card data from current hand
            std::vector<Card*> cardsInHand = hand.CollectAllCardData();
            
            // 2) Move all cards from hand to discard pile
            if (!cardsInHand.empty())
            {
                discard.insert(discard.end(), cardsInHand.begin(), cardsInHand.end());
                cout << "  Discarded " << cardsInHand.size() << " cards from hand" << endl;
            }
            
            // 3) Clear the visual hand
            hand.Clear(objectsList);
            
            // 4) If deck is empty, shuffle discard pile back into deck
            if (deck.empty() && !discard.empty())
            {
                cout << "  Deck is empty. Moving " << discard.size() << " cards from discard to deck" << endl;
                deck.insert(deck.end(), discard.begin(), discard.end());
                discard.clear();
                ShuffleDeck();
            }
            
            // 5) Draw new hand (5 cards)
            const int HAND_SIZE = 5;
            int drawCount = std::min(HAND_SIZE, (int)deck.size());
            
            if (drawCount > 0)
            {
                std::vector<Card*> drawn;
                for (int i = 0; i < drawCount; ++i)
                {
                    drawn.push_back(deck.back());
                    deck.pop_back();
                }
                
                hand.AddCards(drawn, objectsList);
                
                cout << "  Drew " << drawCount << " new cards" << endl;
                for (Card* c : drawn) {
                    if (c != nullptr) {
                        std::cout << "    - " << c->getName() << std::endl;
                    }
                }
            }
            else
            {
                cout << "  No cards available to draw!" << endl;
            }
            
            return; // button handled
        }

        // ------------------------------
        // DISCARD PILE BUTTON (RIGHT) - Left Click does NOTHING
        // ------------------------------
        if (viewDeckButton && IsPointInsideZone(mousePos, viewDeckButton))
        {
            // Do nothing on left-click for discard pile button
            return;
        }

        // ------------------------------
        // DISCARD PILE BUTTON (RIGHT) - Left Click does NOTHING
        // ------------------------------
        if (viewDeckButton && IsPointInsideZone(mousePos, viewDeckButton))
        {
            // Do nothing on left-click for discard pile button
            return;
        }

        // ------------------------------
        // DISCARD PILE BUTTON (RIGHT) - Left Click does NOTHING
        // ------------------------------
        if (viewDeckButton && IsPointInsideZone(mousePos, viewDeckButton))
        {
            // Do nothing on left-click for discard pile button
            return;
        }

        // ------------------------------
        // DISCARD PILE BUTTON (RIGHT) - Left Click does NOTHING
        // ------------------------------
        if (viewDeckButton && IsPointInsideZone(mousePos, viewDeckButton))
        {
            // Do nothing on left-click for discard pile button
            return;
        }

        // ------------------------------
        // DROP ZONES
        // ------------------------------
        CreateDropZones(objectsList);

        hand.UpdateHover(mousePos, false);

        pendingCard = hand.PeekAt(mousePos);
        if (pendingCard) {
            dragStartPos = pendingCard->GetPosition();
        }

        if (testMove) {
            testMoveTarget = glm::vec3(realX, realY, 0);
            testMoveMoving = true;
        }
    }

    // ----------------------------------------------------
    // DRAG
    // ----------------------------------------------------
    if (type == 1)
    {
        if (pendingCard) {
            if (!isDragging) {
                BeginDrag(pendingCard, mousePos);
            }
            else {
                UpdateDrag(mousePos);
            }
        }
        hand.UpdateHover(mousePos, true);
    }
    
    // ----------------------------------------------------
    // RELEASE (Mouse Right Click - type == 2)
    // ----------------------------------------------------
    if (type == 2)
    {
        // Check if right-clicked on RE-DRAW BUTTON to view re-draw deck
        if (reDrawButton && IsPointInsideZone(mousePos, reDrawButton))
        {
            cout << "[UI] View Re-Draw Deck (Right Click)" << endl;
            cout << "=== RE-DRAW DECK CONTENTS ===" << endl;
            cout << "Total cards in re-draw deck: " << deck.size() << endl;
            
            if (!deck.empty())
            {
                for (size_t i = 0; i < deck.size(); ++i)
                {
                    Card* c = deck[i];
                    if (c != nullptr) {
                        std::cout << "  [" << (i + 1) << "] " << c->getName() << std::endl;
                    }
                }
            }
            else
            {
                cout << "  Re-draw deck is empty!" << endl;
            }
            cout << "=============================" << endl;
            
            return; // button handled
        }

        // Check if right-clicked on VIEW-DECK BUTTON to view discard pile
        if (viewDeckButton && IsPointInsideZone(mousePos, viewDeckButton))
        {
            cout << "[UI] View Discard Pile (Right Click)" << endl;
            cout << "=== DISCARD PILE CONTENTS ===" << endl;
            cout << "Total cards in discard pile: " << discard.size() << endl;
            
            if (!discard.empty())
            {
                for (size_t i = 0; i < discard.size(); ++i)
                {
                    Card* c = discard[i];
                    if (c != nullptr) {
                        std::cout << "  [" << (i + 1) << "] " << c->getName() << std::endl;
                    }
                }
            }
            else
            {
                cout << "  Discard pile is empty!" << endl;
            }
            cout << "=============================" << endl;
            
            return; // button handled
        }

        // Handle drag release
        if (isDragging) {
            EndDrag(mousePos);
        }

        isDragging = false;
        isHolding = false;
        draggingCard = nullptr;
        pendingCard = nullptr;

        hand.UpdateHover(mousePos, false);
    }

    if (player) {
        player->SetPosition(glm::vec3(realX, realY, 0));
    }
}


void Level::ShuffleDeck() {
	if (deck.empty()) return;   

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(deck.begin(), deck.end(), g);
}

void Level::DealNewHand(int cardCount) {
    if (cardCount <= 0) {
        return;
    }

    // clear old hand visuals
    hand.Clear(objectsList);

    int drawCount = std::min(cardCount, (int)deck.size());
    std::vector<Card*> drawn;
    for (int i = 0; i < drawCount && !deck.empty(); ++i)
    {
        drawn.push_back(deck.back());
        deck.pop_back();
    }
    if (!drawn.empty())
    {
        hand.AddCards(drawn, objectsList);

        for (Card* c : drawn) {
            if (c != nullptr) {
                std::cout << c->getName() << std::endl;
            }
        }
    }
}


glm::vec3 Level::GridToWorld(int row, int col) const
{
    float x = row * 101.0f - 404.0f;
    float y = col * -105.0f + 352.0f;
    return glm::vec3(x, y, 0.0f);
}

// Drop zones
void Level::CreateDropZones(std::vector<DrawableObject*>& list)
{
    if (dropZonesCreated) return;
    dropZonesCreated = true;

    const float Z = 1.0f;
    const float SIDE_W = 340.0f;
    const float SIDE_H = 520.0f;
    const float MID_W = 550.0f;
    const float MID_H = 260.0f;
    const float BOARD_CENTER_Y = 200.0f;
    const float SIDE_X_OFFSET = 600.0f;
    const float SIDE_Y = BOARD_CENTER_Y;
    const float UPPER_Y = BOARD_CENTER_Y + 180.0f;
    const float BOTTOM_Y = BOARD_CENTER_Y - 180.0f;

    // LEFT
    dropZones[0] = new GameObject();
    dropZones[0]->SetSize(SIDE_W, SIDE_H);
    dropZones[0]->SetPosition(glm::vec3(-SIDE_X_OFFSET, SIDE_Y, Z));
    dropZones[0]->SetColor(1.0f, 0.6f, 0.8f, 0.35f);

    // TOP
    dropZones[1] = new GameObject();
    dropZones[1]->SetSize(MID_W, MID_H);
    dropZones[1]->SetPosition(glm::vec3(0.0f, UPPER_Y, Z));
    dropZones[1]->SetColor(1.0f, 0.6f, 0.8f,0.35f);

    // BOTTOM
    dropZones[2] = new GameObject();
    dropZones[2]->SetSize(MID_W, MID_H);
    dropZones[2]->SetPosition(glm::vec3(0.0f, BOTTOM_Y, Z));
    dropZones[2]->SetColor(1.0f, 0.6f, 0.8f, 0.35f);

    // RIGHT
    dropZones[3] = new GameObject();
    dropZones[3]->SetSize(SIDE_W, SIDE_H);
    dropZones[3]->SetPosition(glm::vec3(SIDE_X_OFFSET, SIDE_Y, Z));
    dropZones[3]->SetColor(1.0f, 0.6f, 0.8f, 0.35f);

    for (int i = 0; i < 4; ++i) {
        list.push_back(dropZones[i]);
        dropZoneSavedPos[i] = dropZones[i]->GetPosition();
        // hide offscreen initially
        dropZones[i]->SetPosition(glm::vec3(
            dropZoneSavedPos[i].x,
            -10000.0f,
            dropZoneSavedPos[i].z
        ));
    }

    dropZonesVisible = false;
}

void Level::ShowDropZones()
{
    if (!dropZonesCreated || dropZonesVisible) return;
    for (int i = 0; i < 4; ++i)
        dropZones[i]->SetPosition(dropZoneSavedPos[i]);
    dropZonesVisible = true;
}

void Level::HideDropZones()
{
    if (!dropZonesCreated || !dropZonesVisible) return;
    for (int i = 0; i < 4; ++i) {
        auto p = dropZoneSavedPos[i];
        dropZones[i]->SetPosition(glm::vec3(p.x, -10000.0f, p.z));
    }
    dropZonesVisible = false;
}



void Level::EnsureBezierSegments(std::vector<DrawableObject*>& list)
{
    if (bezierCreated) return;
    bezierCreated = true;

    bezierSegments.reserve(BEZIER_SEGMENTS);
    for (int i = 0; i < BEZIER_SEGMENTS; ++i) {
        auto* seg = new GameObject();
        seg->SetColor(1.0f, 0.6f, 0.85f);
        seg->SetSize(1.0f, 6.0f);
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 0.0f));
        bezierSegments.push_back(seg);
        list.push_back(seg);
    }
}

void Level::HideBezier()
{
    for (auto* seg : bezierSegments)
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 0.0f));
}

void Level::UpdateBezier(const glm::vec3& P0, const glm::vec3& P1)
{
    if (!bezierCreated) return;

    float midY = 0.5f * (P0.y + P1.y);
    float dir = (midY < screenCenterY) ? 1.0f : -1.0f;

    float liftAmount = 220.0f;
    glm::vec3 mid = 0.5f * (P0 + P1);
    glm::vec3 C = mid + glm::vec3(0.0f, dir * liftAmount, 0.0f);

    for (int i = 0; i < BEZIER_SEGMENTS; ++i) {
        float t0 = (float)i / (float)BEZIER_SEGMENTS;
        float t1 = (float)(i + 1) / (float)BEZIER_SEGMENTS;

        glm::vec3 A = QuadraticBezier(P0, C, P1, t0);
        glm::vec3 B = QuadraticBezier(P0, C, P1, t1);
        glm::vec3 D = B - A;

        float len = glm::length(D);
        auto* seg = bezierSegments[i];

        if (len < 0.001f) {
            seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 500.0f));
            continue;
        }

        glm::vec3 midAB = 0.5f * (A + B);
        float angleRad = std::atan2(D.y, D.x);
        float angleDeg = angleRad * 180.0f / 3.14159265f;

        seg->SetPosition(glm::vec3(midAB.x, midAB.y, 0.0f));
        seg->SetSize(len, 6.0f);
        seg->SetRotate(angleDeg);
    }
}

// Drag & drop of cards

bool Level::IsPointInsideZone(const glm::vec3& p, DrawableObject* zone) const
{
    if (!zone) return false;

    glm::vec3 zpos = zone->GetPosition();
    glm::vec2 zsize = zone->GetSize();

    float halfW = std::abs(zsize.x) * 0.5f;
    float halfH = std::abs(zsize.y) * 0.5f;

    return (p.x >= zpos.x - halfW && p.x <= zpos.x + halfW &&
        p.y >= zpos.y - halfH && p.y <= zpos.y + halfH);
}

int Level::HitDropZone(const glm::vec3& p) const
{
    for (int i = 0; i < 4; ++i)
        if (IsPointInsideZone(p, dropZones[i]))
            return i;
    return -1;
}

void Level::BeginDrag(ImageObject* card, const glm::vec3& mouseWorld)
{
    if (isDragging || !card) return;

    EnsureBezierSegments(objectsList);
    ShowDropZones();

    isDragging = true;
    draggingCard = card;

    dragStartPos = card->GetPosition();
    dragMouseWorld = mouseWorld;

    dragAnchor = dragStartPos;
    draggingCard->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 600.0f));


    UpdateBezier(draggingCard->GetPosition(), mouseWorld);
}

void Level::UpdateDrag(const glm::vec3& mouseWorld)
{
    if (!isDragging || !draggingCard) return;

    dragMouseWorld = mouseWorld;
    draggingCard->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 600.0f));

    glm::vec3 anchor = draggingCard->GetPosition();
    UpdateBezier(anchor, mouseWorld);
}

void Level::EndDrag(const glm::vec3& mouseWorld)
{
    if (!isDragging || !draggingCard) return;

    int dz = HitDropZone(mouseWorld);

    if (dz >= 0)
    {
        Card* cardData = hand.FindCardByImage(draggingCard);

        const char* zoneNames[4] = { "LEFT", "TOP", "BOTTOM", "RIGHT" };
        std::cout << "[DropZone] Dropped in zone: " << zoneNames[dz] << std::endl;

        int moveSteps = 0;

        // store attacks to execute AFTER movement
        struct PendingAttack {
            AttackAction* atk;
            const AttackPattern* pattern;
        };
        std::vector<PendingAttack> pendingAttacks;

        // ---------------- READ CARD ACTIONS ----------------
        if (cardData)
        {
            std::cout << "[Card] " << cardData->getName() << std::endl;
            
            const auto& acts = cardData->getActions();
            for (Action* a : acts)
            {
                if (auto* atk = dynamic_cast<AttackAction*>(a))
                {
                    std::cout << "  AttackAction: " << atk->getValue() << std::endl;

                    const AttackPattern* basePat = dataLoader.getPatternForAction(a);
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
                    moveSteps += mv->getValue();
                    std::cout << "  MoveAction: " << mv->getValue() << std::endl;
                }
            }

            // ---------------- APPLY MOVE ACTION FIRST ----------------
            if (moveSteps > 0 && testGrid)
            {
                std::cout << "Applying MoveAction steps = " << moveSteps
                    << " toward " << zoneNames[dz] << std::endl;

                for (int s = 0; s < moveSteps; ++s)
                {
                    switch (dz)
                    {
                    case 0: // LEFT (same as key 'a')
                        if (nowRow > GridStartRow)
                        {
                            testGrid->Translate(glm::vec3(-(GridWide + distanceBetweenGridX), 0.0f, 0.0f));
                            nowRow--;
                        }
                        break;

                    case 3: // RIGHT (same as key 'd')
                        if (nowRow < GridEndRow - 1)
                        {
                            testGrid->Translate(glm::vec3((GridWide + distanceBetweenGridX), 0.0f, 0.0f));
                            nowRow++;
                        }
                        break;

                    case 1: // TOP (same as key 'w')
                        if (nowCol > GridStartCol)
                        {
                            testGrid->Translate(glm::vec3(0.0f, (GridHigh + distanceBetweenGridY), 0.0f));
                            nowCol--;
                        }
                        break;

                    case 2: // BOTTOM (same as key 's')
                        if (nowCol < GridEndCol - 1)
                        {
                            testGrid->Translate(glm::vec3(0.0f, -(GridHigh + distanceBetweenGridY), 0.0f));
                            nowCol++;
                        }
                        break;
                    }
                }

                std::cout << "Player grid index is now (" << nowRow << ", " << nowCol << ")\n";
            }

            // ---------------- NOW APPLY ALL ATTACK PATTERNS ----------------
            for (const PendingAttack& pa : pendingAttacks)
            {
                AttackAction* atk = pa.atk;
                const AttackPattern* basePat = pa.pattern;

                if (!basePat) {
                    // no pattern linked, just skip pattern damage
                    continue;
                }

                // Orient pattern based on drop zone
                AttackPattern oriented = *basePat;
                int rotateTimes = 0;

                // dz: 0=LEFT, 1=TOP, 2=BOTTOM, 3=RIGHT
                switch (dz)
                {
                case 0: rotateTimes = 3; break; // LEFT = 270° CW
                case 1: rotateTimes = 0; break; // TOP  = 0°
                case 2: rotateTimes = 2; break; // BOTTOM = 180°
                case 3: rotateTimes = 1; break; // RIGHT = 90° CW
                }

                for (int i = 0; i < rotateTimes; ++i) {
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
        }
        else
        {
            std::cout << "[Warning] No Card* bound to this image" << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl;

        // Remove card from hand + render list
        hand.RemoveView(draggingCard);

        auto it = std::find(objectsList.begin(), objectsList.end(), draggingCard);
        if (it != objectsList.end())
            objectsList.erase(it);

        delete draggingCard;
        draggingCard = nullptr;

        // send the card's data to discard pile (for redraw system)
        if (cardData) {
            discard.push_back(cardData);
        }
    }
    else
    {
        // Snap card back
        draggingCard->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 300.0f));
    }

    HideBezier();
    HideDropZones();
    hand.UpdateHover(mouseWorld, false);

    isDragging = false;
    draggingCard = nullptr;
    pendingCard = nullptr;

    std::cout << "End of player turn" << std::endl;
    turnState = TurnState::ENEMY_TURN;
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
        }
    }

    cout << endl;
}
void Level::MoveEnemyTowardPlayer()
{
    if (!enemy) return;

    int er = enemy->getNowRow();
    int ec = enemy->getNowCol();

    int pr = nowRow;  // player's grid row
    int pc = nowCol;  // player's grid col

    int newR = er;
    int newC = ec;

    //  Move one step toward the player 
    if (er < pr) newR = er + 1;
    else if (er > pr) newR = er - 1;
    else if (ec < pc) newC = ec + 1;
    else if (ec > pc) newC = ec - 1;

    // clamp to grid
    newR = std::max(GridStartRow, std::min(newR, GridEndRow - 1));
    newC = std::max(GridStartCol, std::min(newC, GridEndCol - 1));

    // update enemy grid
    enemy->setNowPosition(newR, newC);

    // update sprite position
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
    if (turnState == TurnState::PLAYER_TURN) {
        
    }
    else if (turnState == TurnState::ENEMY_TURN) {

        if (EnemyCanAttackPlayer()) {
            ApplyEnemyAttack();
            
        }
        else {
            MoveEnemyTowardPlayer();
        }

        turnState = TurnState::PLAYER_TURN;
    }
}


