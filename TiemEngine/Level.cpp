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

	hand.CreateVisualHand(5, objectsList);

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

