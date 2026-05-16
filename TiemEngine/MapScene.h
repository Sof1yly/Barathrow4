#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <cmath>

#include "ImageObject.h"
#include "SpriteObject.h"
#include "TextObject.h"
#include "GameObject.h"
#include "DrawableObject.h"
#include "glm.hpp"

class MapScene
{
private:
    std::vector<DrawableObject*> ownedObjects;

    static constexpr int NODE_COUNT = 9;
    ImageObject* nodeIcons[NODE_COUNT]  = {};
    TextObject*  nodeLabels[NODE_COUNT] = {};

    SpriteObject* playerSprite = nullptr;
    GameObject*   fadeOverlay  = nullptr;

    enum class State { FADE_IN, WALKING, PAUSE, FADE_OUT };
    State state = State::FADE_IN;

    float timer = 0.0f;

    static constexpr float FADE_IN_TIME  = 800.0f;   // ms
    static constexpr float PAUSE_TIME    = 700.0f;   // ms at destination
    static constexpr float FADE_OUT_TIME = 800.0f;   // ms
    static constexpr float WALK_SPEED    = 0.28f;    // px per ms

    glm::vec3 nodePositions[NODE_COUNT] = {};
    glm::vec3 walkStart = {};
    glm::vec3 walkEnd   = {};
    float     walkDuration = 1.0f;

    bool active = false;
    bool done   = false;

    static bool IsShopLevel(int level1Based);

public:
    void Open(int fromLevel, int toLevel, std::vector<DrawableObject*>& objectsList);
    void Close(std::vector<DrawableObject*>& objectsList);
    void Update(float dt);

    bool IsActive() const { return active; }
    bool IsDone()   const { return done;   }
};
