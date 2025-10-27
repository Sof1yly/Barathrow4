#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "Button.h"
#include "Card.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "GameDataLoader.h"
#include "Hand.h"
#include "DropZones.h"

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

	hand.CreateVisualHand(5, objectsList);

	zones.UpdateFromViewport(
		GameEngine::GetInstance()->GetDrawAreaWidth(),
		GameEngine::GetInstance()->GetDrawAreaHeight()
	);

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

static inline bool InRect(const Rect& r, float x, float y) {
	return x >= r.xmin && x <= r.xmax && y >= r.ymin && y <= r.ymax;
}



void Level::HandleMouse(int type, int x, int y)
{
	// --- screen -> draw coords (origin center)
	const int   winW = GameEngine::GetInstance()->GetWindowWidth();
	const int   winH = GameEngine::GetInstance()->GetWindowHeight();
	const float drawW = GameEngine::GetInstance()->GetDrawAreaWidth();
	const float drawH = GameEngine::GetInstance()->GetDrawAreaHeight();

	const float realX = (x - winW * 0.5f) * (drawW / winW);
	const float realY = (winH * 0.5f - y) * (drawH / winH);
	const glm::vec3 mousePos(realX, realY, 0.0f);

	// keep drop-zone math up to date
	zones.UpdateFromViewport(drawW, drawH);
	if (dropZonesVisible) UpdateDropZoneVisuals();

	// -------- mouse down --------
	if (type == 0) {
		grabbedObject = nullptr;

		// pick top-most hand card
		for (int i = (int)objectsList.size() - 1; i >= 0; --i) {
			auto* go = dynamic_cast<GameObject*>(objectsList[i]);
			if (!go) continue;
			if (!hand.Owns(go)) continue;
			if (!HitTestGO(go, realX, realY)) continue;

			grabbedObject = go;
			isDragging = true;
			isHolding = true;

			// Cache the card's home NOW. If Owns fails later, we still snap back.
			if (!hand.GetHomeFor(go, grabbedHomePos, grabbedHomeRotDeg)) {
				// fallback: current transform
				grabbedHomePos = go->GetPosition();
				grabbedHomeRotDeg = 0.0f; // or go->GetRotationDeg() if you have it
			}

			ShowDropZones();
			break;
		}
	}

	// -------- mouse move --------
	if (type == 1) {
		if (isDragging && grabbedObject) {
			// move exactly with cursor for reliable drop
			grabbedObject->SetPosition(mousePos);
		}
	}

	// -------- mouse up --------
	if (type == 2) {
		// stop drag first so nothing fights snapback
		isDragging = false;
		isHolding = false;

		GameObject* released = grabbedObject;
		grabbedObject = nullptr;

		if (released) {
			// Decide drop by CARD center (not cursor)
			const glm::vec3 p = released->GetPosition();
			DropZone which = zones.Classify(p.x, p.y);

			// fallback rect checks if Classify is off
			if (which == DropZone::None) {
				if (DropZones::PointIn(zones.left, p.x, p.y)) which = DropZone::Left;
				else if (DropZones::PointIn(zones.right, p.x, p.y)) which = DropZone::Right;
				else if (DropZones::PointIn(zones.top, p.x, p.y)) which = DropZone::Top;
				else if (DropZones::PointIn(zones.bottom, p.x, p.y)) which = DropZone::Bottom;
			}

			if (which == DropZone::None) {
				// FORCE snapback using cached home, even if Hand::Owns failed
				released->SetPosition(grabbedHomePos);
				// If you have rotation API: released->SetRotation(grabbedHomeRotDeg);
				hand.Refan(); // re-layout the fan to be safe
			}
			else {
				// valid zone -> consume and re-fan
				if (hand.Owns(released)) {
					hand.UseCardAndRefan(released);
				}
				auto it = std::find(objectsList.begin(), objectsList.end(),
					static_cast<DrawableObject*>(released));
				if (it != objectsList.end()) objectsList.erase(it);
				delete released;
			}
		}

		// Zones ALWAYS disappear on mouse-up
		HideDropZones();
	}

	// (keep your menu/debug/etc. if needed)
}




bool Level::HitTestGO(GameObject* go, float x, float y) {
	if (!go) return false;
	glm::vec3 pos = go->GetPosition();
	glm::vec2 s = go->GetSize();
	float halfW = s.x * 0.5f, halfH = s.y * 0.5f;
	const float pad = 50.0f; // make grabbing easier
	return (x >= pos.x - halfW - pad && x <= pos.x + halfW + pad &&y >= pos.y - halfH - pad && y <= pos.y + halfH + pad);
}

GameObject* Level::CreateZoneFromRect(const Rect& r, glm::vec3 color) {
	auto* go = new GameObject();
	const float w = r.xmax - r.xmin, h = r.ymax - r.ymin;
	const float cx = (r.xmax + r.xmin) * 0.5f, cy = (r.ymax + r.ymin) * 0.5f;
	go->SetSize(w, h);
	go->SetPosition({ cx, cy, 0 });
	go->SetColor(color.r, color.g, color.b); // r,g,b
	return go;
}

void Level::ShowDropZones() {
	if (dropZonesVisible) return;
	dropZonesVisible = true;
	dropZoneGOs = {
		CreateZoneFromRect(zones.left,   {0,1,1}),
		CreateZoneFromRect(zones.right,  {0,1,1}),
		CreateZoneFromRect(zones.top,    {0,1,1}),
		CreateZoneFromRect(zones.bottom, {0,1,1})
	};
	for (auto* go : dropZoneGOs) objectsList.push_back(go);
}

void Level::HideDropZones() {
	if (!dropZonesVisible) return;
	dropZonesVisible = false;
	for (auto* go : dropZoneGOs) {
		auto it = std::find(objectsList.begin(), objectsList.end(),
			static_cast<DrawableObject*>(go));
		if (it != objectsList.end()) objectsList.erase(it);
		delete go;
	}
	dropZoneGOs.clear();
}

void Level::UpdateDropZoneVisuals() {
	if (!dropZonesVisible || dropZoneGOs.size() != 4) return;
	auto apply = [](GameObject* go, const Rect& r) {
		const float w = r.xmax - r.xmin, h = r.ymax - r.ymin;
		const float cx = (r.xmax + r.xmin) * 0.5f, cy = (r.ymax + r.ymin) * 0.5f;
		go->SetSize(w, h);
		go->SetPosition({ cx, cy, 0 });
		};
	apply(dropZoneGOs[0], zones.left);
	apply(dropZoneGOs[1], zones.right);
	apply(dropZoneGOs[2], zones.top);
	apply(dropZoneGOs[3], zones.bottom);
}