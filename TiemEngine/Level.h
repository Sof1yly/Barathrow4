#pragma once
#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "Button.h"
#include "GameDataLoader.h"
#include "Hand.h"



class Level
{
private:
	vector<DrawableObject*> objectsList;
	GameObject * player;
	GameObject* testMove;
	GameObject* testGrid;
	ImageObject* mainMenu;
	GameObject* grabbedObject = nullptr;
	GameObject* draggableObject = nullptr;
	GameObject* dropZones[4];

	bool dropZonesCreated = false;
	bool dropZonesVisible = false;
	glm::vec3 dropZoneSavedPos[4];

	void CreateDropZones(std::vector<DrawableObject*>& objectsList);
	void ShowDropZones();
	void HideDropZones();

	GameDataLoader dataLoader;
	Hand hand;

	glm::vec3 testMoveTarget;
	glm::vec3 grabbedTarget = glm::vec3(0.0f);
	glm::vec3 grabOffset = glm::vec3(0.0f);
	bool testMoveMoving = false;
	bool isDragging = false;
	bool isHolding = false;

	int nowRow = 0;
	int nowCol = 0;

public:
	virtual void LevelLoad();
	virtual void LevelInit();
	virtual void LevelUpdate();
	virtual void LevelDraw();
	virtual void LevelFree();
	virtual void LevelUnload();
	virtual void HandleKey(char key);
	virtual void HandleMouse(int type, int x, int y);
};
