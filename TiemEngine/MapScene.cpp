#include "MapScene.h"

bool MapScene::IsShopLevel(int level1Based)
{
    return level1Based == 4 || level1Based == 9;
}

void MapScene::Open(int fromLevel, int toLevel, std::vector<DrawableObject*>& objectsList)
{
    if (active) return;
    active   = true;
    done     = false;
    viewOnly = false;
    state    = State::FADE_IN;
    timer    = 0.0f;

    // Default to win-transition timings
    activeFadeInTime  = FADE_IN_TIME;
    activeFadeOutTime = FADE_OUT_TIME;

    // Node positions: 9 nodes evenly spaced left-to-right, centered on screen
    const float startX  = -700.0f;
    const float spacing = 175.0f;
    const float nodeY   =   30.0f;

    for (int i = 0; i < NODE_COUNT; i++)
        nodePositions[i] = glm::vec3(startX + i * spacing, nodeY, 0.0f);

    int fromIdx = std::max(0, std::min(fromLevel - 1, NODE_COUNT - 1));
    int toIdx   = std::max(0, std::min(toLevel   - 1, NODE_COUNT - 1));

    // Dark background — covers all game content underneath
    auto* bg = new GameObject();
    bg->SetSize(1920.0f, 1080.0f);
    bg->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    bg->SetColor(0.04f, 0.04f, 0.08f, 1.0f);
    ownedObjects.push_back(bg);
    objectsList.push_back(bg);

    // Connecting lines between nodes
    for (int i = 0; i < NODE_COUNT - 1; i++)
    {
        float midX  = (nodePositions[i].x + nodePositions[i + 1].x) * 0.5f;
        float lineW = spacing - 88.0f;

        auto* line = new GameObject();
        line->SetSize(lineW, 6.0f);
        line->SetPosition(glm::vec3(midX, nodeY, 0.0f));
        line->SetColor(0.35f, 0.35f, 0.35f, 0.9f);
        ownedObjects.push_back(line);
        objectsList.push_back(line);
    }

    // Node icons and labels
    for (int i = 0; i < NODE_COUNT; i++)
    {
        const char* texPath = IsShopLevel(i + 1)
            ? "../Resource/Texture/MapIcon/Shop.PNG"
            : "../Resource/Texture/MapIcon/Normal.PNG";

        auto* icon = new ImageObject();
        icon->SetTexture(texPath);
        icon->SetSize(80.0f, -80.0f);
        icon->SetPosition(nodePositions[i]);
        ownedObjects.push_back(icon);
        objectsList.push_back(icon);
        nodeIcons[i] = icon;

        SDL_Color labelCol = (i == toIdx)
            ? SDL_Color{ 255, 215, 50, 255 }
            : SDL_Color{ 170, 170, 170, 255 };

        auto* label = new TextObject();
        label->LoadText(std::to_string(i + 1), labelCol, 22);
        label->SetPosition(glm::vec3(nodePositions[i].x, nodePositions[i].y - 68.0f, 0.0f));
        ownedObjects.push_back(label);
        objectsList.push_back(label);
        nodeLabels[i] = label;
    }

    // Player sprite — starts at fromLevel node
    const float playerY = nodeY + 8.0f;
    walkStart = glm::vec3(nodePositions[fromIdx].x, playerY, 0.0f);
    walkEnd   = glm::vec3(nodePositions[toIdx  ].x, playerY, 0.0f);

    float dist   = std::abs(walkEnd.x - walkStart.x);
    walkDuration = (dist > 0.0f) ? (dist / WALK_SPEED) : PAUSE_TIME;

    playerSprite = new SpriteObject("../Resource/Texture/Player_sprite2.png", 9, 16);
    playerSprite->SetSize(100.0f, -100.0f);
    playerSprite->SetPosition(walkStart);
    playerSprite->SetAnimationLoop(0, 6, 2, 800);   // idle facing right
    ownedObjects.push_back(playerSprite);
    objectsList.push_back(playerSprite);

    // Fade overlay — added last so it renders on top of everything
    fadeOverlay = new GameObject();
    fadeOverlay->SetSize(1920.0f, 1080.0f);
    fadeOverlay->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    fadeOverlay->SetColor(0.0f, 0.0f, 0.0f, 1.0f);  // starts fully black
    ownedObjects.push_back(fadeOverlay);
    objectsList.push_back(fadeOverlay);
}

void MapScene::OpenViewOnly(int currentLevel, std::vector<DrawableObject*>& objectsList)
{
    Open(currentLevel, currentLevel, objectsList);
    viewOnly          = true;
    activeFadeInTime  = VIEW_FADE_IN_TIME;
    activeFadeOutTime = VIEW_FADE_OUT_TIME;

    // Back button with "Return" label — inserted BEFORE the fade overlay so the
    // fade reveals them rather than them rendering on top of the overlay.
    backBtnPos  = glm::vec3(0.0f, -370.0f, 0.0f);
    backBtnSize = glm::vec2(200.0f, 70.0f);

    backButton = new ImageObject();
    backButton->SetTexture("../Resource/Texture/UI/SettingUI/Back.png");
    backButton->SetSize(backBtnSize.x, -backBtnSize.y);
    backButton->SetPosition(backBtnPos);
    ownedObjects.push_back(backButton);
    objectsList.insert(objectsList.end() - 1, backButton);  // before fadeOverlay

    returnText = new TextObject();
    SDL_Color white = { 255, 255, 255, 255 };
    returnText->LoadText("Return", white, 28);
    returnText->SetPosition(backBtnPos);
    ownedObjects.push_back(returnText);
    objectsList.insert(objectsList.end() - 1, returnText);  // before fadeOverlay
}

void MapScene::HandleClose()
{
    if (!active || done) return;
    if (state == State::VIEW_IDLE)
    {
        state = State::FADE_OUT;
        timer = 0.0f;
    }
}

bool MapScene::HandleClick(float worldX, float worldY)
{
    if (!active || !viewOnly || done) return false;
    if (state != State::VIEW_IDLE) return false;

    float halfW = backBtnSize.x * 0.5f;
    float halfH = backBtnSize.y * 0.5f;

    if (worldX >= backBtnPos.x - halfW && worldX <= backBtnPos.x + halfW &&
        worldY >= backBtnPos.y - halfH && worldY <= backBtnPos.y + halfH)
    {
        HandleClose();
        return true;
    }
    return false;
}

void MapScene::Update(float dt)
{
    if (!active || done) return;

    timer += dt;

    switch (state)
    {
    case State::FADE_IN:
    {
        float alpha = 1.0f - (timer / activeFadeInTime);
        alpha = std::max(0.0f, std::min(1.0f, alpha));
        if (fadeOverlay) fadeOverlay->SetColor(0.0f, 0.0f, 0.0f, alpha);

        if (timer >= activeFadeInTime)
        {
            if (fadeOverlay) fadeOverlay->SetColor(0.0f, 0.0f, 0.0f, 0.0f);
            if (viewOnly)
            {
                state = State::VIEW_IDLE;
            }
            else
            {
                if (playerSprite) playerSprite->SetAnimationLoop(1, 12, 4, 150);
                state = State::WALKING;
            }
            timer = 0.0f;
        }
        break;
    }
    case State::WALKING:
    {
        float t       = std::min(1.0f, timer / walkDuration);
        glm::vec3 pos = walkStart + (walkEnd - walkStart) * t;
        if (playerSprite) playerSprite->SetPosition(pos);

        if (timer >= walkDuration)
        {
            if (playerSprite)
            {
                playerSprite->SetPosition(walkEnd);
                playerSprite->SetAnimationLoop(0, 6, 2, 800);
            }
            state = State::PAUSE;
            timer = 0.0f;
        }
        break;
    }
    case State::PAUSE:
    {
        if (timer >= PAUSE_TIME)
        {
            state = State::FADE_OUT;
            timer = 0.0f;
        }
        break;
    }
    case State::VIEW_IDLE:
        // Waiting for Back button click or key press
        break;

    case State::FADE_OUT:
    {
        float alpha = std::min(1.0f, timer / activeFadeOutTime);
        if (fadeOverlay) fadeOverlay->SetColor(0.0f, 0.0f, 0.0f, alpha);

        if (timer >= activeFadeOutTime)
            done = true;
        break;
    }
    }
}

void MapScene::Close(std::vector<DrawableObject*>& objectsList)
{
    for (auto* obj : ownedObjects)
    {
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    ownedObjects.clear();

    for (int i = 0; i < NODE_COUNT; i++)
    {
        nodeIcons[i]  = nullptr;
        nodeLabels[i] = nullptr;
    }
    playerSprite = nullptr;
    fadeOverlay  = nullptr;
    backButton   = nullptr;
    returnText   = nullptr;
    active = false;
    done   = false;
}
