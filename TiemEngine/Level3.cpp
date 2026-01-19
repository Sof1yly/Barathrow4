#include "Level3.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"

void Level3::LevelLoad()
{
	SquareMeshVbo * square = new SquareMeshVbo();
	square->LoadData();
	GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);

	SpriteMeshVbo * sprite = new SpriteMeshVbo();
	sprite->LoadData();
	GameEngine::GetInstance()->AddMesh(SpriteMeshVbo::MESH_NAME, sprite);

	cout << "Load Level 3" << endl;
}

void Level3::LevelInit()
{
	GameObject * obj = new GameObject();
	obj->SetColor(1.0, 0.0, 0.0);
	objectsList.push_back(obj);

	GameObject* obj2 = new GameObject();
	obj2->SetColor(0.0, 1.0, 0.0);
	objectsList.push_back(obj2);
	obj2->SetPosition(glm::vec3(2.5f, 2.5f, 0.0f));

	player = obj;

	ImageObject * img = new ImageObject();
	img->SetSize(2.0f, -2.0f);
	img->SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
	img->SetTexture("../Resource/Texture/penguin.png");
	objectsList.push_back(img);

	ImageObject* img2 = new ImageObject();
	img2->SetSize(2.0f, -2.0f);
	img2->SetPosition(glm::vec3(-2.0f, -2.0f, 0.0f));
	img2->SetTexture("../Resource/Texture/doro.png");
	objectsList.push_back(img2);

	ImageObject* img3 = new ImageObject();
	img3->SetSize(4.0f, -4.0f);
	img3->SetPosition(glm::vec3(-0.0f, -0.0f, 0.0f));
	img3->SetTexture("../Resource/Texture/doro.png");
	objectsList.push_back(img3);

	SpriteObject * sprite = new SpriteObject("../Resource/Texture/TestSprite.png", 4, 7);
	sprite->SetSize(2.0f, -2.0f);
	sprite->SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	sprite->SetAnimationLoop(0, 0, 27, 50);
	objectsList.push_back(sprite);

	cout << "Init Level" << endl;
}

void Level3::LevelUpdate()
{
	//cout << "Update Level" << endl;
	int deltaTime = GameEngine::GetInstance()->GetDeltaTime();
	for (DrawableObject* obj : objectsList) {
		obj->Update(deltaTime);
	}
	
}

void Level3::LevelDraw()
{
	GameEngine::GetInstance()->Render(objectsList);
	//cout << "Draw Level" << endl;
}

void Level3::LevelFree()
{
	for (DrawableObject* obj : objectsList) {
		delete obj;
	}
	objectsList.clear();
	cout << "Free Level" << endl;
}

void Level3::LevelUnload()
{
	GameEngine::GetInstance()->ClearMesh();
	cout << "Unload Level" << endl;
}

void Level3::HandleKey(char key)
{

	switch (key)
	{
		case 'w': player->Translate(glm::vec3(0, 0.3, 0)); break;
		case 's': player->Translate(glm::vec3(0, -0.3, 0)); break;
		case 'a': player->Translate(glm::vec3(-0.3, 0, 0)); break;
		case 'd': player->Translate(glm::vec3(0.3, 0, 0)); break;
		case 'q': GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT; ; break;
		case 'r': GameData::GetInstance()->gGameStateNext = GameState::GS_RESTART; ; break;
		case 'e': GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1; ; break;
	}
}

void Level3::HandleMouse(int type, int x, int y)
{
	float realX, realY;

	// Calculate Real X Y 
	realX = (x/100.0)-3.0;
	realY = ((y/100.0)-3.0)*(-1.0);

	GameEngine::GetInstance()->GetWindowHeight();
	GameEngine::GetInstance()->GetWindowWidth();

	cout << "X : " << x << "	Y" << y << endl;
	cout << "real X : " << realX << "	real Y" << realY << endl;

	player->SetPosition(glm::vec3(realX, realY, 0));
}