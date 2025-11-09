#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

#include "GameEngine.h"
#include "GameObject.h"
#include "GameData.h"
#include "ImageObject.h"
#include "SpriteObject.h"
#include "Button.h"
#include "AttackPattern.h"
#include "GameDataLoader.h"
#include "Hand.h"

class Level
{
private:
    // Render list
    std::vector<DrawableObject*> objectsList;

    // Basic objects
    GameObject* player = nullptr;
    GameObject* testMove = nullptr;
    ImageObject* testGrid = nullptr;   // player marker on grid
    ImageObject* mainMenu = nullptr;

    // Generic dragging (non-card)
    GameObject* grabbedObject = nullptr;
    glm::vec3   grabbedTarget = glm::vec3(0.0f);
    glm::vec3   grabOffset = glm::vec3(0.0f);

    // Grid config
    int   GridStartRow = 0;
    int   GridEndRow = 9;
    int   GridStartCol = 0;
    int   GridEndCol = 5;
    float GridWide = 90.0f;
    float GridHigh = 84.0f;
    int   distanceBetweenGridX = 11;
    int   distanceBetweenGridY = 21;

    int nowRow = 0;
    int nowCol = 0;

    // Movement
    glm::vec3 testMoveTarget = glm::vec3(0.0f);
    bool      testMoveMoving = false;

    // Hand & data
    GameDataLoader dataLoader;
    Hand           hand;

    // Drop zones
    GameObject* dropZones[4] = { nullptr, nullptr, nullptr, nullptr };
    bool        dropZonesCreated = false;
    bool        dropZonesVisible = false;
    glm::vec3   dropZoneSavedPos[4];

    // Drag & card leash
    bool        isDragging = false;   // card drag (also used by grabbedObject)
    bool        isHolding = false;
    ImageObject* draggingCard = nullptr;
    ImageObject* pendingCard = nullptr; // selected on mouse-down before drag
    glm::vec3   dragStartPos = glm::vec3(0.0f);
    glm::vec3   dragMouseWorld = glm::vec3(0.0f);
    glm::vec3   dragAnchor = glm::vec3(0.0f);   // anchor on card for rope

    // Bezier rope (segments)
    static const int BEZIER_SEGMENTS = 32;
    std::vector<GameObject*> bezierSegments;
    bool  bezierCreated = false;
    float screenCenterY = 0.0f;

    // Attack pattern
    std::vector<AttackPattern> patterns;
    AttackPattern rotatedPattern;
    int currentPatternIndex = 0;
    int currentRotation = 0;  // 0,90,180,270

    // ---- helpers: drop zones ----
    void CreateDropZones(std::vector<DrawableObject*>& list);
    void ShowDropZones();
    void HideDropZones();

    // ---- helpers: bezier rope ----
    void EnsureBezierSegments(std::vector<DrawableObject*>& list);
    void HideBezier();
    void UpdateBezier(const glm::vec3& P0, const glm::vec3& P1);

    // ---- helpers: hit test ----
    bool IsPointInsideZone(const glm::vec3& p, GameObject* zone) const;
    int  HitDropZone(const glm::vec3& p) const;

    // ---- helpers: drag flow ----
    void BeginDrag(ImageObject* card, const glm::vec3& mouseWorld);
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

    // keep for compatibility if something else calls it
    virtual void CreateCard(int cardCount, std::vector<DrawableObject*>& objectsList);
};
