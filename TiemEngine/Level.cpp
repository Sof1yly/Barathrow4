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

	CreateCard(5, objectsList);

	GameDataLoader loader;

	string error;

	bool loaded = loader.loadFromFile("D:/Assignment/Y2_2026/Y2_Project/ActionTest.txt", &error);

	if (!loaded) {
		cerr << "Error: " << error << endl;
		return;
	}
	cout << "Loaded card: " << endl;
	for (Card* card : loader.getCards()) {
		cout << "Card: " << card->getName() << endl;
		card->do_action();
		
	}

	

	//GameObject* obj7 = new GameObject(); //1
	//obj7->SetColor(0.0, 5.0, 5.0);
	//obj7->SetPosition(glm::vec3(125.0f, -245.0f, 0.0f));
	//obj7->SetSize(220.0f, 335.0f);
	//obj7->SetRotate(-23.5f);
	//objectsList.push_back(obj7);

	//GameObject* obj4 = new GameObject();//2
	//obj4->SetColor(0.0, 5.0, 0.0);
	//obj4->SetPosition(glm::vec3(0.80f, -2.25f, 0.0f));
	//obj4->SetSize(100.0f, 160.0f);
	//obj4->SetRotate(-15.0f);
	//objectsList.push_back(obj4);


	//ImageObject* img2 = new ImageObject();
	//img2->SetSize(2.0f, -2.0f);
	//img2->SetPosition(glm::vec3(-2.0f, -2.0f, 0.0f));
	//img2->SetTexture("../Resource/Texture/doro.png");
	//objectsList.push_back(img2);

	//SpriteObject * sprite = new SpriteObject("../Resource/Texture/TestSprite.png", 4, 7);
	//sprite->SetSize(2.0f, -2.0f);
	//sprite->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	//sprite->SetAnimationLoop(0, 0, 27, 50);
	//objectsList.push_back(sprite);

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
	float realX, realY;

	int winW = GameEngine::GetInstance()->GetWindowWidth();
	int winH = GameEngine::GetInstance()->GetWindowHeight();

	float scaleW = GameEngine::GetInstance()-> GetDrawAreaWidth();
	float scaleH = GameEngine::GetInstance()-> GetDrawAreaHeight();


	realX = (x - winW / 2) * (scaleW / winW);
	realY = (winH / 2 - y) * (scaleH / winH);
	

	GameEngine::GetInstance()->GetWindowHeight();
	GameEngine::GetInstance()->GetWindowWidth();

	glm::vec3 mousePos(realX, realY, 0.0f);
	

	if (type == 0) {
		cout << "Mouse Pressed\n";
		if (draggableObject) {
			glm::vec3 pos = draggableObject->GetPosition();
			glm::vec2 s = draggableObject->GetSize();
			float halfW = s.x * 0.5f;
			float halfH = s.y * 0.5f;

			const float grabPadding = 30.0f;

			if (mousePos.x >= pos.x - halfW - grabPadding && mousePos.x <= pos.x + halfW + grabPadding &&
				mousePos.y >= pos.y - halfH - grabPadding && mousePos.y <= pos.y + halfH + grabPadding)
			{
				grabbedObject = draggableObject;
				grabbedTarget = mousePos;
				isDragging = true;
			}
			else {
				isDragging = false;
				grabbedObject = nullptr;
			}
		}
	}

	if (type == 1) {
		if (isDragging && grabbedObject == draggableObject) {
			isHolding = true;

			glm::vec3 current = grabbedObject->GetPosition();
			glm::vec3 diff = mousePos - current;

			float followSpeed = 0.3f;
			grabbedObject->Translate(diff * followSpeed);
		}
	}

	if (type == 2) {
		std::cout << "Mouse Released\n";
		isDragging = false;
		isHolding = false;
		grabbedObject = nullptr;
	}

	if (type == 0 || type == 1) {
		testMoveTarget = glm::vec3(realX, realY, 0.0f);
		testMoveMoving = true;
	}
	

	if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470 && type == 0 ) {
		cout << "MenuButton Down" << endl;
		if (Button::getMenu() == false) {
			Button::setMenu(true);
			mainMenu->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		}
		else {
			Button::setMenu(false);
			mainMenu->SetPosition(glm::vec3(0.0f, 20000.0f, 0.0f));
		}
	}
	/*
	if (realX >= -3 && realX <= -1.5 && realY <= -1.5 && realY >= -2.5) {
		cout << "Doro" << endl;
	}*/
	cout << "X : " << x << "	Y" << y << endl;
	cout << "real X : " << realX << "	real Y" << realY << endl;

	player->SetPosition(glm::vec3(realX, realY, 0));
}

void Level::CreateCard(int cardCount, std::vector<DrawableObject*>& objectsList)
{
	if (cardCount <= 0) return;

	const float cardW = 220.0f, cardH = 335.0f;
	const float handY = -540.0f;

	// Circle controls
	const float baseRadius = 1000.0f;              // bigger = flatter arc
	const float centerDrop = 120.0f;              // pushes the middle further down
	const float R = baseRadius + centerDrop;

	// Circle center -> lowest rim point sits at 'handY'
	const float Cx = 0.0f;
	const float Cy = handY - R;

	float sepFactor = 0.65f;                      // around 0.5 - 0.7 is good

	// Optional clamp of total span in arc-length pixels (0=off)
	const float maxArcSpanPixels = 0.0f;          // e.g., 1400.0f to cap width

	// Fan rotation:
	const float fanStrength = 1.0f;               // 1 = fully tangent, 0 = upright, 0.5 = half
	const float baseTiltDeg = 0.0f;               // constant offset if you want a slight lean

	const float sink = 4.0f;                      // sink the bottom a few pixels past the rim
	const float PI = 3.1415926535f;
	const float RAD2DEG = 180.0f / PI;

	// --------- Spacing as ARC LENGTH -> angle step ---------
	float arcSpacing = cardW * sepFactor;                   // pixels per neighbor along arc
	if (maxArcSpanPixels > 0.0f && cardCount > 1) {
		float needed = arcSpacing * float(cardCount - 1);
		if (needed > maxArcSpanPixels) {
			arcSpacing = maxArcSpanPixels / float(cardCount - 1);
		}
	}
	float stepDeg = (arcSpacing / R) * RAD2DEG;


	float spanDeg = (cardCount > 1) ? stepDeg * float(cardCount - 1) : 0.0f;
	float startAngleDeg = -0.5f * spanDeg;

	// Build left -> right; newest gets drawn on top
	for (int i = 0; i < cardCount; ++i)
	{

		float angleDeg = startAngleDeg + float(i) * stepDeg;
		float theta = angleDeg * (PI / 180.0f);

		// Rim point (where the card bottom touches)
		float rimX = Cx + std::sinf(theta) * R;
		float rimY = Cy + std::cosf(theta) * R;

		// Radial normal (from center -> rim) and tangent (fan direction)
		float nx = std::sinf(theta);
		float ny = std::cosf(theta);
		float tx = ny;
		float ty = -nx;

		// Place sprite center: move half card height outward from the rim (minus sink)
		float px = rimX + nx * (cardH * 0.5f - sink);
		float py = rimY + ny * (cardH * 0.5f - sink);

		// Fan rotation: interpolate between upright (0) and tangent (atan2 of tangent)
		float tangentDeg = std::atan2f(ty, tx) * RAD2DEG;  // angle facing along the arc
		float rotDeg = baseTiltDeg + fanStrength * tangentDeg;

		// Create & push
		GameObject* card = new GameObject();
		card->SetSize(cardW, cardH);
		card->SetPosition(glm::vec3(px, py, 0.0f));
		card->SetRotate(rotDeg); // radians? -> rotDeg * (PI/180)

		// debug tint
		if (i % 3 == 0) {
			card->SetColor(5.0f, 0.0f, 0.0f);
		}
		else if (i % 3 == 1) {
			card->SetColor(0.0f, 5.0f, 0.0f);
		}
		else {
			card->SetColor(0.0f, 0.0f, 5.0f);
		}

		objectsList.push_back(card);
	}
}