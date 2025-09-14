#pragma once
#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "Button.h"

class Level
{
private:
	vector<DrawableObject*> objectsList;
	GameObject * player;
	GameObject* testMove;
	ImageObject* mainMenu;

	glm::vec3 testMoveTarget;
	bool testMoveMoving = false;

public:
	virtual void LevelLoad();
	virtual void LevelInit();
	virtual void LevelUpdate();
	virtual void LevelDraw();
	virtual void LevelFree();
	virtual void LevelUnload();

	virtual void CreateCard(int cardCount, vector<DrawableObject*>& objectsList);
	virtual void HandleKey(char key);
	virtual void HandleMouse(int type, int x, int y);
};
