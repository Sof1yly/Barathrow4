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

    // 2) Attack patterns
    patterns = {
        AttackPattern::fromGrid({
            ".X.",
            "XXX",
            ".X."
        }, 'X'),

        AttackPattern::fromGrid({
            "XXX",
            "XXX",
            "XXX"
        }, 'X'),

        AttackPattern::fromGrid({
            "..X..",
            ".XXX.",
            "XXXXX",
            ".XXX.",
            "..X.."
        }, 'X'),

        AttackPattern::fromGrid({
            "...XX",
        }, 'X'),
    };

    currentPatternIndex = 0;
    currentRotation = 0;
    rotatedPattern = patterns[currentPatternIndex];

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

    // Hand + Drop zones + Card data
    std::string error;
    if (!dataLoader.loadFromFile("../Resource/GameData/ActionTest.txt", &error)) {
        std::cerr << "Error loading card data: " << error << std::endl;
    }

    // Create visual hand using loaded cards
    hand.CreateVisualHand(5, objectsList, dataLoader.getCards());

    // Drop zones
    CreateDropZones(objectsList);

    std::cout << "Init Level" << std::endl;
}

void Level::LevelUpdate()
{
    int deltaTime = GameEngine::GetInstance()->GetDeltaTime();

    for (auto* obj : objectsList)
        obj->Update((float)deltaTime);

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
    case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL2;  break;
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

		cout << "Using pattern #" << currentPatternIndex + 1 << endl;
		for (auto& cell : attacks) {
			int x = cell.first.x;
			int y = cell.first.y;

			if (x < 0 || x >= GridEndRow || y < 0 || y >= GridEndCol) {
				cout << "Skipped out-of-bound attack at (" << x << ", " << y << ")\n";
				continue;
			}

			cout << "attack at (" << x << ", " << y << ")\n";
		}
		cout << endl;
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

    //Mouse Hover
    if (type == 3) {
        hand.UpdateHover(mousePos, isDragging);
        return;
    }

    // Mouse down
    if (type == 0)
    {
        // Menu toggle
        if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470)
        {
            if (!Button::getMenu()) {
                Button::setMenu(true);
                if (mainMenu) mainMenu->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            }
            else {
                Button::setMenu(false);
                if (mainMenu) mainMenu->SetPosition(glm::vec3(0.0f, 20000.0f, 0.0f));
            }
        }

        CreateDropZones(objectsList);

        hand.UpdateHover(mousePos, false);

        pendingCard = hand.PeekAt(mousePos);


        if (pendingCard) {
            dragStartPos = pendingCard->GetPosition();
        }

        if (testMove) {
            testMoveTarget = glm::vec3(realX, realY, 0.0f);
            testMoveMoving = true;
        }
    }

    // Mouse drag
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

        // while dragging, disable hover effects
        hand.UpdateHover(mousePos, true);
    }

    // Mouse up
    if (type == 2)
    {
        if (isDragging) {
            EndDrag(mousePos);
        }

        isDragging = false;
        isHolding = false;
        draggingCard = nullptr;
        pendingCard = nullptr;

        // after release, allow hover again at the new mouse position
        hand.UpdateHover(mousePos, false);
    }

    if (player) {
        player->SetPosition(glm::vec3(realX, realY, 0.0f));
    }
}

// Drop zones


void Level::CreateDropZones(std::vector<DrawableObject*>& list)
{
    if (dropZonesCreated) return;
    dropZonesCreated = true;

    const float Z = 400.0f;
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
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 500.0f));
        bezierSegments.push_back(seg);
        list.push_back(seg);
    }
}

void Level::HideBezier()
{
    for (auto* seg : bezierSegments)
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 500.0f));
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

        seg->SetPosition(glm::vec3(midAB.x, midAB.y, 500.0f));
        seg->SetSize(len, 6.0f);
        seg->SetRotate(angleDeg);
    }
}

// Drag & drop of cards

bool Level::IsPointInsideZone(const glm::vec3& p, GameObject* zone) const
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

        if (cardData)
        {
            std::cout << "[Card] " << cardData->getName() << std::endl;

            const auto& acts = cardData->getActions();
            for (Action* a : acts)
            {
                if (auto* atk = dynamic_cast<AttackAction*>(a))
                    std::cout << "  AttackAction: " << atk->getValue() << std::endl;
                else if (auto* mv = dynamic_cast<MoveAction*>(a))
                    std::cout << "  MoveAction: " << mv->getValue() << std::endl;
                else
                    std::cout << "  (Unknown action type)" << std::endl;
            }
        }
        else
        {
            std::cout << "[Warning] No Card* bound to this image" << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl;

        // remove from hand + render list
        hand.RemoveView(draggingCard);

        auto it = std::find(objectsList.begin(), objectsList.end(), draggingCard);
        if (it != objectsList.end())
            objectsList.erase(it);

        delete draggingCard;
        draggingCard = nullptr;
    }
    else
    {
        // snap back to start
        draggingCard->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 300.0f));
    }

    HideBezier();
    HideDropZones();

    hand.UpdateHover(mouseWorld, false);

    isDragging = false;
    draggingCard = nullptr;
    pendingCard = nullptr;
}





