#pragma once
#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "Button.h"
#include "GameDataLoader.h"
#include "Hand.h"
#include "DropZones.h"
#include <vector>


class Level
{
private:
    std::vector<DrawableObject*> objectsList;
    GameObject* player = nullptr;
    GameObject* testMove = nullptr;
    GameObject* testGrid = nullptr;
    ImageObject* mainMenu = nullptr;

    // Dragging
    GameObject* grabbedObject = nullptr;
    glm::vec3 grabbedTarget = glm::vec3(0.0f);
    glm::vec3 grabOffset = glm::vec3(0.0f);
    bool isDragging = false;
    bool isHolding = false;

  
    GameObject* draggableObject = nullptr; 
    glm::vec3 testMoveTarget;
    bool testMoveMoving = false;


    int nowRow = 0;
    int nowCol = 0;

    GameDataLoader dataLoader;
    Hand hand;
    DropZones zones;

    std::vector<GameObject*> dropZoneGOs;
    bool dropZonesVisible = false;
    glm::vec3 grabbedHomePos = glm::vec3(0);
    float grabbedHomeRotDeg = 0.0f;

    GameObject* CreateZoneFromRect(const Rect& rect, glm::vec3 color);
    void UpdateDropZoneVisuals();
    void ShowDropZones();
    void HideDropZones();

    static bool HitTestGO(GameObject* go, float x, float y);

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
