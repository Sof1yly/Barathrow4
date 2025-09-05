#include "Level.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "Button.h"

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

	ImageObject * img = new ImageObject();
	img->SetSize(200.0f, -200.0f);
	img->SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
	img->SetTexture("../Resource/Texture/penguin.png");
	objectsList.push_back(img);

	CreateCard(5, objectsList);

	//GameObject* obj7 = new GameObject(); //1
	//obj7->SetColor(0.0, 5.0, 5.0);
	//obj7->SetPosition(glm::vec3(1.25f, -2.45f, 0.0f));
	//obj7->SetSize(1.0f, 1.6f);
	//obj7->SetRotate(-23.5f);
	//objectsList.push_back(obj7);

	//GameObject* obj4 = new GameObject();//2
	//obj4->SetColor(0.0, 5.0, 0.0);
	//obj4->SetPosition(glm::vec3(0.80f, -2.25f, 0.0f));
	//obj4->SetSize(1.0f, 1.6f);
	//obj4->SetRotate(-15.0f);
	//objectsList.push_back(obj4);

	//GameObject* obj3 = new GameObject(); //3
	//obj3->SetColor(0.0, 0.0, 5.0);
	//obj3->SetPosition(glm::vec3(0.0f, -2.15f, 0.0f));
	//obj3->SetSize(1.0f, 1.6f);
	//obj3->SetRotate(0.0f);
	//objectsList.push_back(obj3);

	//GameObject* obj6 = new GameObject();//4
	//obj6->SetColor(5.0, 5.0, 0.0);
	//obj6->SetPosition(glm::vec3(-1.25f, -2.45f, 0.0f));
	//obj6->SetSize(1.0f, 1.6f);
	//obj6->SetRotate(23.5f);
	//objectsList.push_back(obj6);

	//GameObject* obj5 = new GameObject();//5
	//obj5->SetColor(5.0, 0.0, 0.0);
	//obj5->SetPosition(glm::vec3(-0.80f, -2.25f, 0.0f));
	//obj5->SetSize(1.0f, 1.6f);
	//obj5->SetRotate(15.0f);
	//objectsList.push_back(obj5);


	//ImageObject* img2 = new ImageObject();
	//img2->SetSize(2.0f, -2.0f);
	//img2->SetPosition(glm::vec3(-2.0f, -2.0f, 0.0f));
	//img2->SetTexture("../Resource/Texture/doro.png");
	//objectsList.push_back(img2);

	SpriteObject * sprite = new SpriteObject("../Resource/Texture/TestSprite.png", 4, 7);
	sprite->SetSize(2.0f, -2.0f);
	sprite->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	sprite->SetAnimationLoop(0, 0, 27, 50);
	objectsList.push_back(sprite);

	cout << "Init Level" << endl;
}

void Level::LevelUpdate()
{
	//cout << "Update Level" << endl;
	int deltaTime = GameEngine::GetInstance()->GetDeltaTime();
	for (DrawableObject* obj : objectsList) {
		obj->Update(deltaTime);
	}

	if(Button::getMenu()==true) {
		cout << "Go to Menu" << endl;
	}
	else {
		cout << "Not Go to Menu" << endl;
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
		case 'w': player->Translate(glm::vec3(0, 0.3, 0)); break;
		case 's': player->Translate(glm::vec3(0, -0.3, 0)); break;
		case 'a': player->Translate(glm::vec3(-0.3, 0, 0)); break;
		case 'd': player->Translate(glm::vec3(0.3, 0, 0)); break;
		case 'q': GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT; ; break;
		case 'r': GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART; ; break;
		case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL2; ; break; 
	}
}

void Level::HandleMouse(int type, int x, int y)
{
	float realX, realY;

	int winW = GameEngine::GetInstance()->GetWindowWidth();
	int winH = GameEngine::GetInstance()->GetWindowHeight();

	realX = ((x - winW)+(winW/2));
	realY = (((y-winH)*-1)-(winH/2));
	

	GameEngine::GetInstance()->GetWindowHeight();
	GameEngine::GetInstance()->GetWindowWidth();

	if (realX >= 850 && realX <= 900 && realY <= 530 && realY >= 470) {
		cout << "MenuButton Down" << endl;
		if (Button::getMenu() == false) {
			Button::setMenu(true);
		}
		else {
			Button::setMenu(false);
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

void Level::CreateCard(int cardCount, vector<DrawableObject*>& objectsList) {
	float baseY = -2.0f;  
	float dx = 0.8f;    
	float curve = 0.1f;    
	float rotStep = 7.5f;    

	float center = (cardCount - 1) / 2.0f;

	for (int i = 0; i < cardCount; i++) {
		float offset = i - center;

		float x = offset * (dx + 0.1f * fabs(offset));
		float y = baseY - curve * (offset * offset);

		float rot = -(offset * rotStep);

	
		GameObject* card = new GameObject();
		card->SetPosition(glm::vec3(x, y, 0.0f));
		card->SetRotate(rot);
		card->SetSize(1.0f, 1.6f); //Don't for got to fix

	
		card->SetColor((i % 3 == 0) ? 5.0f : 0.0f,(i % 3 == 1) ? 5.0f : 0.0f,(i % 3 == 2) ? 5.0f : 0.0f);

		objectsList.push_back(card);
	}
}