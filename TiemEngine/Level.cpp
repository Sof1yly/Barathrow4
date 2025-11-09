#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "Button.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "GameDataLoader.h"
#include "AttackPattern.h"



static inline glm::vec3 QuadraticBezier(
	const glm::vec3& P0,
	const glm::vec3& C,
	const glm::vec3& P1,
	float t)
{
	float u = 1.0f - t;
	return (u * u) * P0 + 2.0f * u * t * C + (t * t) * P1;
}

void Level::LevelLoad()
{
	SquareMeshVbo * square = new SquareMeshVbo();
	square->LoadData();
	GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);

	SpriteMeshVbo * sprite = new SpriteMeshVbo();
	sprite->LoadData();
	GameEngine::GetInstance()->AddMesh(SpriteMeshVbo::MESH_NAME, sprite);

	cout << "Load Level" << endl;


	
}

void Level::LevelInit()
{
	// Create a grid
	for (int i = GridStartRow; i < GridEndRow; i++) {
		for (int j = GridStartCol; j < GridEndCol; j++) {
			ImageObject* tile = new ImageObject();
			tile->SetTexture("../Resource/Texture/tile.png");
			tile->SetSize(GridWide, GridHigh);
			tile->SetPosition(glm::vec3(i * 101.0f - 404.0f, j * -105.0f + 352.0f, 0.0f)); //Set distance between tiles
			//If y pixel not correct, change 394 to 352
			objectsList.push_back(tile);
		}
	}
	//End of grid

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


	ImageObject* testgrid = new ImageObject();
	testgrid->SetSize(84.0f, -90.0f);
	testgrid->SetPosition(glm::vec3(-404.0f, 352.0f, 0.0f));
	testgrid->SetTexture("../Resource/Texture/player.png");
	objectsList.push_back(testgrid);

	testGrid = testgrid;

	GameObject * obj = new GameObject();
	obj->SetColor(1.0, 0.0, 0.0);
	obj->SetSize(200.0, 200.0);
	objectsList.push_back(obj);

	GameObject* obj2 = new GameObject();
	obj2->SetColor(0.0, 1.0, 0.0);
	obj2->SetSize(50.0, 50.0);
	objectsList.push_back(obj2);
	obj2->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));

	player = obj;

	GameObject* obj3 = new GameObject();
	obj3->SetColor(0.0, 0.0, 1.0);
	obj3->SetSize(100.0, 100.0);
	objectsList.push_back(obj3);
	obj3->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

	testMove = obj3;

	/*GameObject* obj4 = new GameObject();
	obj4->SetColor(1.0, 1.0, 0.0);
	obj4->SetSize(75.0, 75.0);
	objectsList.push_back(obj4);
	obj4->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

	draggableObject = obj4;*/
	grabbedObject = nullptr;


	testMoveTarget = testMove->GetPosition();

	ImageObject * img = new ImageObject();
	img->SetSize(50.0f, -50.0f);
	img->SetPosition(glm::vec3(900.0f, 500.0f, 0.0f));
	img->SetTexture("../Resource/Texture/menu.png");
	objectsList.push_back(img);

	ImageObject * MenuUI = new ImageObject();
	MenuUI->SetSize(1000.0f, -400.0f);
	MenuUI->SetPosition(glm::vec3(0.0f, 10000.0f, 0.0f));
	MenuUI->SetTexture("../Resource/Texture/MainMenu.png");
	objectsList.push_back(MenuUI);
	
	mainMenu = MenuUI;

	//mainMenu = MenuUI;

	hand.CreateVisualHand(5, objectsList);
	CreateDropZones(objectsList);
	GameDataLoader loader;

	string error;

	bool loaded = loader.loadFromFile("../Resource/GameData/ActionTest.txt", &error);

	if (!loaded) {
		cerr << "Error: " << error << endl;
		return;
	}
	cout << "Loaded card: " << endl;
	for (Card* card : loader.getCards()) {
		cout << "Card: " << card->getName() << endl;
		card->do_action();
		
	}

	cout << "Init Level" << endl;
}

void Level::LevelUpdate()
{
	//cout << "Update Level" << endl;
	int deltaTime = GameEngine::GetInstance()->GetDeltaTime();
	for (DrawableObject* obj : objectsList) {
		obj->Update(deltaTime);
	}

	if (isDragging && grabbedObject && !isHolding) {
		glm::vec3 current = grabbedObject->GetPosition();
		glm::vec3 diff = grabbedTarget - current;
		float dist = glm::length(diff);

		if (dist > 1.0f) {
			glm::vec3 dir = glm::normalize(diff);
			float speedPixelsPerSecond = 600.0f;
			float step = speedPixelsPerSecond * (deltaTime / 1000.0f);
			grabbedObject->Translate(dir * step);
		}
		else {
			grabbedObject->SetPosition(grabbedTarget);
			isDragging = false;
			grabbedObject = nullptr;
		}
	}

	if (testMoveMoving) {
		glm::vec3 currentPos = testMove->GetPosition();

		// Direction toward target
		glm::vec3 dir = testMoveTarget - currentPos;
		float dist = glm::length(dir);

		if (dist > 1.0f) { // still far away
			dir = glm::normalize(dir);
			float speed = 0.5f * deltaTime; // adjust speed
			testMove->Translate(dir * speed);
		}
		else {
			testMove->SetPosition(testMoveTarget); // snap to target
			testMoveMoving = false;
		}
	}

	if(Button::getMenu()==true) {
		//cout << "Go to Menu" << endl; // Test Mouse Click
	}
	else {
		//cout << "Not Go to Menu" << endl;
	}
	
}

void Level::LevelDraw()
{
	GameEngine::GetInstance()->Render(objectsList);
	//cout << "Draw Level" << endl;
}

void Level::LevelFree()
{
	for (DrawableObject* obj : objectsList) {
		delete obj;
	}
	objectsList.clear();
	cout << "Free Level" << endl;
}

void Level::LevelUnload()
{
	GameEngine::GetInstance()->ClearMesh();
	cout << "Unload Level" << endl;
}

void Level::HandleKey(char key)
{

	switch (key)
	{
		//case 'w': testGrid->Translate(glm::vec3(0, 100.0, 0)); break;
		//case 's': testGrid->Translate(glm::vec3(0, -100.0, 0)); break;
		//case 'a': testGrid->Translate(glm::vec3(-100.0, 0, 0)); break;
		//case 'd': testGrid->Translate(glm::vec3(100.0, 0, 0)); break;
	case 'q': GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT; ; break;
	case 'r': GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART; ; break;
	case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL2; ; break;
	}

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
			cout << "attack at (" << cell.first.x << ", " << cell.first.y << ")\n";
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
	float realX, realY;

	int winW = GameEngine::GetInstance()->GetWindowWidth();
	int winH = GameEngine::GetInstance()->GetWindowHeight();

	float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
	float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

	realX = (x - winW / 2) * (scaleW / winW);
	realY = (winH / 2 - y) * (scaleH / winH);

	glm::vec3 mousePos(realX, realY, 0.0f);


	screenCenterY = 0.0f;


	if (type == 0)
	{
		std::cout << "Mouse Pressed\n";

		// Menu button toggle (unchanged)
		if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470)
		{
			std::cout << "MenuButton Down" << std::endl;
			if (!Button::getMenu())
			{
				Button::setMenu(true);
				mainMenu->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			}
			else
			{
				Button::setMenu(false);
				mainMenu->SetPosition(glm::vec3(0.0f, 20000.0f, 0.0f));
			}
		}

		CreateDropZones(objectsList);


		pendingCard = hand.PeekAt(mousePos);

		// DO NOT show dropzones yet
		// DO NOT call TrySelectAt (no pop/no scale)

		testMoveTarget = glm::vec3(realX, realY, 0.0f);
		testMoveMoving = true;
	}

	// ---------- type 1: mouse is held/moved while still down ----------
	if (type == 1)
	{
		// if we have a card from mousedown:
		if (pendingCard)
		{
			// if drag not yet started, start now
			if (!isDragging)
			{
				BeginDrag(pendingCard, mousePos);
			}
			else
			{
				UpdateDrag(mousePos);
			}
		}
	}

	// ---------- type 2: button released ----------
	if (type == 2)
	{
		std::cout << "Mouse Released\n";

		if (isDragging)
		{
			EndDrag(mousePos);
		}

		// clear state
		isDragging = false;
		isHolding = false;
		grabbedObject = nullptr;
		pendingCard = nullptr;
	}

	// debug + player follow (unchanged)
	std::cout << "X : " << x << "    Y : " << y << std::endl;
	std::cout << "real X : " << realX << "    real Y : " << realY << std::endl;

	player->SetPosition(glm::vec3(realX, realY, 0));
}


void Level::CreateDropZones(std::vector<DrawableObject*>& objectsList)
{
	if (dropZonesCreated) return;
	dropZonesCreated = true;

	const float Z = 400.0f; // render depth (above board, below selected card)

	// Tall side zones
	const float SIDE_W = 340.0f;   // wide side panels
	const float SIDE_H = 520.0f;   // tall side panels

	// Upper / Bottom zones around the board
	const float MID_W = 550.0f;   // wide (almost as wide as board+padding)
	const float MID_H = 260.0f;   // chunky horizontal bar


	const float BOARD_CENTER_Y = 200.0f;



	const float SIDE_X_OFFSET = 600.0f;

	// Vertical position for side panels: centered around board
	const float SIDE_Y = BOARD_CENTER_Y;

	// Upper zone goes above the board
	const float UPPER_Y = BOARD_CENTER_Y + 180.0f;

	// Bottom zone goes below the board but still above the hand cards
	const float BOTTOM_Y = BOARD_CENTER_Y - 180.0f;


	// ----- LEFT -----
	dropZones[0] = new GameObject();
	dropZones[0]->SetSize(SIDE_W, SIDE_H);
	dropZones[0]->SetPosition({
		-SIDE_X_OFFSET,  // big push to left side
		SIDE_Y,
		Z
		});
	dropZones[0]->SetRotate(0.0f);
	dropZones[0]->SetColor(1.0f, 0.6f, 0.8f); // pastel pink

	// ----- UPPER -----
	dropZones[1] = new GameObject();
	dropZones[1]->SetSize(MID_W, MID_H);
	dropZones[1]->SetPosition({
		0.0f,
		UPPER_Y,
		Z
		});
	dropZones[1]->SetRotate(0.0f);
	dropZones[1]->SetColor(1.0f, 0.6f, 0.8f);

	// ----- BOTTOM -----
	dropZones[2] = new GameObject();
	dropZones[2]->SetSize(MID_W, MID_H);
	dropZones[2]->SetPosition({
		0.0f,
		BOTTOM_Y,
		Z
		});
	dropZones[2]->SetRotate(0.0f);
	dropZones[2]->SetColor(1.0f, 0.6f, 0.8f);

	// ----- RIGHT -----
	dropZones[3] = new GameObject();
	dropZones[3]->SetSize(SIDE_W, SIDE_H);
	dropZones[3]->SetPosition({
		SIDE_X_OFFSET,   // big push to right side
		SIDE_Y,
		Z
		});
	dropZones[3]->SetRotate(0.0f);
	dropZones[3]->SetColor(1.0f, 0.6f, 0.8f);


	//
	// Add to draw list and hide initially
	//
	for (int i = 0; i < 4; ++i) {
		objectsList.push_back(dropZones[i]);

		// Save their true visible position
		dropZoneSavedPos[i] = dropZones[i]->GetPosition();

		// Move off-screen (hidden) until player starts dragging a card
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
	if (!dropZonesCreated) return;     // not created yet
	if (dropZonesVisible) return;      // already visible — do nothing

	for (int i = 0; i < 4; ++i) {
		if (!dropZones[i]) continue;
		dropZones[i]->SetPosition(dropZoneSavedPos[i]);
	}
	dropZonesVisible = true;
}

void Level::HideDropZones()
{
	if (!dropZonesCreated) return;     // not created yet
	if (!dropZonesVisible) return;     // already hidden — do nothing

	for (int i = 0; i < 4; ++i) {
		if (!dropZones[i]) continue;
		auto p = dropZoneSavedPos[i];
		dropZones[i]->SetPosition(glm::vec3(p.x, -10000.0f, p.z)); // move off-screen
	}
	dropZonesVisible = false;
}


// Create visual dots for the curve if not created yet
void Level::EnsureBezierDots(std::vector<DrawableObject*>& objectsListRef)
{
	if (bezierCreated) return;
	bezierCreated = true;

	bezierDots.reserve(BEZIER_DOT_COUNT);

	for (int i = 0; i < BEZIER_DOT_COUNT; ++i)
	{
		GameObject* d = new GameObject();
		d->SetSize(4.0f, 4.0f);
		d->SetColor(1.0f, 0.6f, 0.85f); // pink
		d->SetRotate(0.0f);

		// start hidden offscreen
		d->SetPosition({ 99999.0f, 99999.0f, 500.0f }); // 500 = above drop zones

		bezierDots.push_back(d);
		objectsListRef.push_back(d);
	}

	// keep synced
	objectsList = objectsListRef;
}

void Level::ShowBezier()
{
	for (auto* d : bezierDots)
	{
		if (!d) continue;
		glm::vec3 p = d->GetPosition();
		d->SetPosition({ p.x, p.y, 500.0f });
	}
}

void Level::HideBezier()
{
	for (auto* d : bezierDots)
	{
		if (!d) continue;
		d->SetPosition({ 99999.0f, 99999.0f, 500.0f });
	}
}

// Updates the curve from dragAnchor (P0) to mouse (P1)
void Level::UpdateBezier(const glm::vec3& P0, const glm::vec3& P1)
{
	if (!bezierCreated) return;

	// which way should the arc bend?
	float midY = 0.5f * (P0.y + P1.y);
	float dir = (midY < screenCenterY) ? 1.0f : -1.0f;

	float liftAmount = 220.0f;
	glm::vec3 mid = 0.5f * (P0 + P1);
	glm::vec3 lift = glm::vec3(0.0f, dir * liftAmount, 0.0f);
	glm::vec3 C = mid + lift;

	for (int i = 0; i < BEZIER_DOT_COUNT; ++i)
	{
		float t = (float)i / (float)(BEZIER_DOT_COUNT - 1);
		glm::vec3 p = QuadraticBezier(P0, C, P1, t);

		// curve should be above zones
		bezierDots[i]->SetPosition({ p.x, p.y, 500.0f });
	}
}


bool Level::IsPointInsideZone(const glm::vec3& p, GameObject* zone) const
{
	if (!zone) return false;

	glm::vec3 zpos = zone->GetPosition();
	glm::vec2 zsize = zone->GetSize();

	float halfW = zsize.x * 0.5f;
	float halfH = zsize.y * 0.5f;

	return (
		p.x >= zpos.x - halfW && p.x <= zpos.x + halfW &&
		p.y >= zpos.y - halfH && p.y <= zpos.y + halfH
		);
}

int Level::HitDropZone(const glm::vec3& p) const
{
	for (int i = 0; i < 4; ++i)
	{
		if (IsPointInsideZone(p, dropZones[i]))
			return i;
	}
	return -1;
}



void Level::BeginDrag(GameObject* card, const glm::vec3& mouseWorld)
{
	if (isDragging || !card) return;

	// make sure curve visual exists
	EnsureBezierDots(objectsList);

	// show helpers now that we are ACTUALLY dragging
	ShowDropZones(); // zones at Z=400
	ShowBezier();    // dots at Z=500

	isDragging = true;
	draggingCard = card;
	dragStartPos = card->GetPosition();
	dragMouseWorld = mouseWorld;

	// anchor near the top of the card
	glm::vec2 cardSize = card->GetSize();
	dragAnchor = dragStartPos + glm::vec3(0.0f, cardSize.y * 0.5f, 0.0f);

	// bring card in front of everything
	draggingCard->SetPosition({ dragStartPos.x, dragStartPos.y, 600.0f });

	// draw first leash frame
	UpdateBezier(dragAnchor, mouseWorld);
}

void Level::UpdateDrag(const glm::vec3& mouseWorld)
{
	if (!isDragging || !draggingCard) return;

	dragMouseWorld = mouseWorld;

	// card leans toward mouse, but we clamp it so it doesn't fly
	glm::vec3 newPos = dragStartPos;

	float rawDY = mouseWorld.y - dragStartPos.y;
	float dy = rawDY;
	if (dy < -40.0f) dy = -40.0f;
	if (dy > 80.0f) dy = 80.0f;
	newPos.y += dy;

	// keep on top visually
	draggingCard->SetPosition({ newPos.x, newPos.y, 600.0f });

	UpdateBezier(dragAnchor, mouseWorld);
}

void Level::EndDrag(const glm::vec3& mouseWorld)
{
	if (!isDragging || !draggingCard) return;

	HideBezier();
	HideDropZones();

	int dz = HitDropZone(mouseWorld);
	if (dz >= 0)
	{

		auto it = std::find(objectsList.begin(), objectsList.end(), draggingCard);
		if (it != objectsList.end()) {
			objectsList.erase(it);
		}

		delete draggingCard;
		draggingCard = nullptr;
	}
	else
	{

		draggingCard->SetPosition({
			dragStartPos.x,
			dragStartPos.y,
			300.0f // hand layer depth after cancel
			});
	}

	isDragging = false;
	draggingCard = nullptr;
	pendingCard = nullptr;
}
