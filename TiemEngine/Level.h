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
	GameObject* grabbedObject = nullptr;
	GameObject* draggableObject = nullptr;

	glm::vec3 testMoveTarget;
	glm::vec3 grabbedTarget = glm::vec3(0.0f);
	glm::vec3 grabOffset = glm::vec3(0.0f);
	bool testMoveMoving = false;
	bool isDragging = false;
	bool isHolding = false;

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
