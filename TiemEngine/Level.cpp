#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "Button.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "GameDataLoader.h"


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
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			GameObject* tile = new GameObject();
			if ((i + j) % 2 == 0) {
				tile->SetColor(0.8f, 0.8f, 0.8f);
			}
			else {
				tile->SetColor(0.2f, 0.2f, 0.2f);
			}
			tile->SetSize(100.0f, 100.0f);
			tile->SetPosition(glm::vec3(i * 100.0f - 200.0f, j * -100.0f + 300.0f, 0.0f)); //Set distance between tiles
			objectsList.push_back(tile);
		}
	}
	//End of grid

	GameObject* testgrid = new GameObject();
	testgrid->SetColor(1.0f, 0.5f, 1.0f);
	testgrid->SetSize(100.0f, 100.0f);
	testgrid->SetPosition(glm::vec3(-200.0f, 300.0f, 0.0f));
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

	GameObject* obj4 = new GameObject();
	obj4->SetColor(1.0, 1.0, 0.0);
	obj4->SetSize(75.0, 75.0);
	objectsList.push_back(obj4);
	obj4->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

	draggableObject = obj4;
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
	size_t before = objectsList.size();
	CreateCard(5, objectsList);
	size_t after = objectsList.size();

	for (size_t i = before; i < after; i++) {
		handCards.push_back(static_cast<GameObject*>(objectsList[i]));
	}

	InitDropZones();
	LayoutHand();

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
	if (key == 'w' && nowCol != 0) {
		testGrid->Translate(glm::vec3(0, 100.0, 0));
		nowCol--;
	}
	else if (key == 's' && nowCol != 4) {
		testGrid->Translate(glm::vec3(0, -100.0, 0));
		nowCol++;
	}
	else if (key == 'a'&&nowRow!=0) {
		testGrid->Translate(glm::vec3(-100.0, 0, 0));
		nowRow--;
	}
	else if (key == 'd'&&nowRow!=4) {
		testGrid->Translate(glm::vec3(100.0, 0, 0));
		nowRow++;
	}
}

void Level::HandleMouse(int type, int x, int y)
{
	float realX, realY;

	int   winW = GameEngine::GetInstance()->GetWindowWidth();
	int   winH = GameEngine::GetInstance()->GetWindowHeight();
	float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
	float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

	// screen -> world
	realX = (x - winW / 2) * (scaleW / winW);
	realY = (winH / 2 - y) * (scaleH / winH);
	glm::vec3 mousePos(realX, realY, 0.0f);

	// helper to know if a GameObject* is one of the hand cards
	auto isHandCard = [&](GameObject* g)->bool {
		return std::find(handCards.begin(), handCards.end(), g) != handCards.end();
	};

	if (type == 0)
	{
		std::cout << "Mouse Pressed\n";

		bool cardGrabbed = false;
		if (!isDragging) {
			if (GameObject* picked = PickTopHandCard(mousePos)) {
				draggableObject = picked;       // reuse existing fields
				grabbedObject = picked;
				isDragging = true;
				isHolding = false;
				grabOffset = mousePos - picked->GetPosition(); // keep cursor offset
				grabbedTarget = mousePos;
				BringToFront(draggableObject);
				cardGrabbed = true;
			}
		}

		if (!cardGrabbed && draggableObject) {
			glm::vec3 pos = draggableObject->GetPosition();
			glm::vec2 s = draggableObject->GetSize();
			float halfW = s.x * 0.5f;
			float halfH = s.y * 0.5f;
			const float grabPadding = 50.0f;

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

	if (type == 1)
	{
		if (isDragging && grabbedObject)
		{
			if (isHandCard(grabbedObject)) {
				// Cards follow the mouse exactly, keeping the initial offset
				grabbedObject->SetPosition(mousePos - grabOffset);
			}
			else if (grabbedObject == draggableObject) {
				// Your original smooth-follow drag for non-card object(s)
				isHolding = true;
				glm::vec3 current = grabbedObject->GetPosition();
				glm::vec3 diff = mousePos - current;
				float followSpeed = 0.3f;
				grabbedObject->Translate(diff * followSpeed);
			}
		}
	}

	if (type == 2)
	{
		std::cout << "Mouse Released\n";

		if (isDragging && grabbedObject && isHandCard(grabbedObject)) {
			bool inDrop =
				dzLeft.contains(mousePos) ||dzRight.contains(mousePos) ||dzTop.contains(mousePos) ||dzBottom.contains(mousePos);
			if (inDrop) {
				RemoveCardFromHand(grabbedObject);
				draggableObject = nullptr;  // card no longer exists
				LayoutHand();
			}
			else {
				LayoutHand();
			}
		}

		isDragging = false;
		isHolding = false;
		grabbedObject = nullptr;
		return;
	}

	if (type == 0 || type == 1) {
		testMoveTarget = glm::vec3(realX, realY, 0.0f);
		testMoveMoving = true;
	}

	if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470 && type == 0) {
		std::cout << "MenuButton Down" << std::endl;
		if (Button::getMenu() == false) {
			Button::setMenu(true);
			mainMenu->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		}
		else {
			Button::setMenu(false);
			mainMenu->SetPosition(glm::vec3(0.0f, 20000.0f, 0.0f));
		}
	}

	std::cout << "X : " << x << "    Y" << y << std::endl;
	std::cout << "real X : " << realX << "    real Y" << realY << std::endl;

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

void Level::InitDropZones()
{
	// World-space rectangles; tweak to match your pink mock overlay.
	dzLeft = Rect{ -960.0f, -520.0f, -140.0f,  380.0f };
	dzRight = Rect{ 520.0f,  960.0f, -140.0f,  380.0f };
	dzTop = Rect{ -360.0f,  360.0f,   80.0f,  380.0f };
	dzBottom = Rect{ -360.0f,  360.0f, -300.0f,  -40.0f };
}

void Level::BringToFront(DrawableObject* obj)
{
	auto it = std::find(objectsList.begin(), objectsList.end(), obj);
	if (it != objectsList.end()) { objectsList.erase(it); objectsList.push_back(obj); }
}

void Level::RemoveCardFromHand(GameObject* card)
{
	auto it = std::find(handCards.begin(), handCards.end(), card);
	if (it != handCards.end()) handCards.erase(it);

	auto it2 = std::find(objectsList.begin(), objectsList.end(),
		static_cast<DrawableObject*>(card));
	if (it2 != objectsList.end()) objectsList.erase(it2);

	delete card;
}

GameObject* Level::PickTopHandCard(const glm::vec3& p)
{
	auto inside = [](GameObject* c, const glm::vec3& pt) {
		glm::vec3 pos = c->GetPosition();
		glm::vec2 s = c->GetSize();
		float hx = s.x * 0.5f, hy = s.y * 0.5f;
		// simple AABB pick (rotation ignored; fine for fanned look)
		return pt.x >= pos.x - hx && pt.x <= pos.x + hx &&
			pt.y >= pos.y - hy && pt.y <= pos.y + hy;
		};
	for (int i = (int)handCards.size() - 1; i >= 0; --i)
		if (inside(handCards[i], p)) return handCards[i];
	return nullptr;
}

// Re-fan cards (same geometry as in CreateCard)
void Level::LayoutHand()
{
	const int n = (int)handCards.size();
	if (n <= 0) return;

	const float cardW = 220.0f, cardH = 335.0f;
	const float handY = -540.0f;
	const float baseRadius = 1000.0f, centerDrop = 120.0f;
	const float R = baseRadius + centerDrop;
	const float Cx = 0.0f, Cy = handY - R;
	const float sink = 4.0f, sep = 0.65f;
	const float PI = 3.1415926535f, RAD2DEG = 180.0f / PI;

	float arcSpacing = cardW * sep;
	float stepDeg = (arcSpacing / R) * RAD2DEG;
	float spanDeg = (n > 1 ? stepDeg * float(n - 1) : 0.0f);
	float startDeg = -0.5f * spanDeg;

	for (int i = 0; i < n; ++i) {
		float a = (startDeg + i * stepDeg) * (PI / 180.0f);
		float nx = std::sinf(a), ny = std::cosf(a);
		float rimX = Cx + nx * R, rimY = Cy + ny * R;
		float px = rimX + nx * (cardH * 0.5f - sink);
		float py = rimY + ny * (cardH * 0.5f - sink);
		float tx = ny, ty = -nx;
		float rotDeg = std::atan2f(ty, tx) * RAD2DEG;

		auto* c = handCards[i];
		c->SetSize(cardW, cardH);
		c->SetPosition(glm::vec3(px, py, 0));
		c->SetRotate(rotDeg);
	}
}
