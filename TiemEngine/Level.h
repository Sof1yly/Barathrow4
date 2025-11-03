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
    GameObject* player;
    GameObject* testMove;
    GameObject* testGrid;
    ImageObject* mainMenu;
    GameObject* grabbedObject = nullptr;     // legacy
    GameObject* draggableObject = nullptr;   // legacy
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
    glm::vec3 grabbedTarget = glm::vec3(0.0f); // legacy
    glm::vec3 grabOffset = glm::vec3(0.0f);    // legacy
    bool testMoveMoving = false;

    bool isDragging = false;  
    bool isHolding = false;   
    int nowRow = 0;
    int nowCol = 0;


    GameObject* pendingCard = nullptr;   
    GameObject* draggingCard = nullptr;  

    glm::vec3 dragStartPos = glm::vec3(0.0f);
    glm::vec3 dragAnchor = glm::vec3(0.0f);
    glm::vec3 dragMouseWorld = glm::vec3(0.0f);

  
    static const int BEZIER_DOT_COUNT = 28;
    std::vector<GameObject*> bezierDots;
    bool bezierCreated = false;

 
    float screenCenterY = 0.0f;

    // helpers for curve + dropzone
    void EnsureBezierDots(std::vector<DrawableObject*>& objectsList);
    void ShowBezier();
    void HideBezier();
    void UpdateBezier(const glm::vec3& P0, const glm::vec3& P1);

    bool IsPointInsideZone(const glm::vec3& p, GameObject* zone) const;
    int  HitDropZone(const glm::vec3& p) const;

    void BeginDrag(GameObject* card, const glm::vec3& mouseWorld);
    void UpdateDrag(const glm::vec3& mouseWorld);
    void EndDrag(const glm::vec3& mouseWorld);

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
