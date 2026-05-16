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

    // View-only back button
    ImageObject* backButton = nullptr;
    TextObject*  returnText = nullptr;
    glm::vec3    backBtnPos  = {};
    glm::vec2    backBtnSize = {};

    enum class State { FADE_IN, WALKING, PAUSE, FADE_OUT, VIEW_IDLE };
    State state = State::FADE_IN;

    bool viewOnly = false;

    float timer = 0.0f;

    // Win-transition timings
    static constexpr float FADE_IN_TIME  = 800.0f;
    static constexpr float PAUSE_TIME    = 700.0f;
    static constexpr float FADE_OUT_TIME = 800.0f;
    static constexpr float WALK_SPEED    = 0.28f;

    // View-only timings (faster)
    static constexpr float VIEW_FADE_IN_TIME  = 250.0f;
    static constexpr float VIEW_FADE_OUT_TIME = 250.0f;

    // Active timings (set at open time)
    float activeFadeInTime  = FADE_IN_TIME;
    float activeFadeOutTime = FADE_OUT_TIME;

    glm::vec3 nodePositions[NODE_COUNT] = {};
    glm::vec3 walkStart = {};
    glm::vec3 walkEnd   = {};
    float     walkDuration = 1.0f;

    bool active = false;
    bool done   = false;

    static bool IsShopLevel(int level1Based);

public:
    // Win transition: player walks from fromLevel to toLevel, then fade out
    void Open(int fromLevel, int toLevel, std::vector<DrawableObject*>& objectsList);

    // View-only: faster fade, player idle at currentLevel node, Back button to close
    void OpenViewOnly(int currentLevel, std::vector<DrawableObject*>& objectsList);

    void Close(std::vector<DrawableObject*>& objectsList);
    void Update(float dt);

    // Called by Level when player clicks Back button (view-only mode)
    void HandleClose();

    // Hit-tests the Back button; calls HandleClose if hit. Returns true if handled.
    bool HandleClick(float worldX, float worldY);

    bool IsActive()   const { return active;  }
    bool IsDone()     const { return done;     }
    bool IsViewOnly() const { return viewOnly; }
};
