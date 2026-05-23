#include "EventPage.h"
#include "SquareMeshVbo.h"
#include "SpriteMeshVbo.h"
#include "GameStateList.h"
#include "GameData.h"
#include "SaveSystem.h"
#include "SoundManager.h"

void EventPage::GetRealMousePos(int x, int y, float& rx, float& ry) const
{
    int   winW   = GameEngine::GetInstance()->GetWindowWidth();
    int   winH   = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();
    rx = (x - winW  / 2.0f) * (scaleW / winW);
    ry = (winH / 2.0f - y)  * (scaleH / winH);
}

void EventPage::TransitionToLevel(EventScene::EffectType effect)
{
    GameData::GetInstance()->eventEffectType = static_cast<int>(effect);
    GameData::GetInstance()->gGameStateNext  = GameState::GS_LEVEL1;
}

void EventPage::LevelLoad()
{
    auto* square = new SquareMeshVbo();
    square->LoadData();
    GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);

    auto* sprite = new SpriteMeshVbo();
    sprite->LoadData();
    GameEngine::GetInstance()->AddMesh(SpriteMeshVbo::MESH_NAME, sprite);
}

void EventPage::LevelInit()
{
    // If this is a "Continue" load, skip the event and go straight to the level
    if (SaveSystem::pendingLoad)
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1;
        return;
    }

    // Game world background
    auto* bg = new ImageObject();
    bg->SetTexture("../Resource/Texture/BG/Floor1_FHD.PNG");
    bg->SetSize(1920.0f, -1080.0f);
    bg->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    objectsList.push_back(bg);

    eventScene.Open(objectsList);

    // Fade overlay — above all EventScene objects (max z ~715), starts invisible
    fadeOverlay = new ImageObject();
    fadeOverlay->SetTexture("../Resource/Texture/UI/Menu/Fade.png");
    fadeOverlay->SetColor(0.0f, 0.0f, 0.0f);
    fadeOverlay->SetSize(1920.0f, -1080.0f);
    fadeOverlay->SetPosition(glm::vec3(0.0f, 0.0f, 800.0f));
    fadeOverlay->SetAlpha(0.0f);
    objectsList.push_back(fadeOverlay);
}

void EventPage::LevelUpdate()
{
    int dt = GameEngine::GetInstance()->GetDeltaTime();

    for (auto* obj : objectsList)
        obj->Update((float)dt);

    if (eventScene.IsActive())
    {
        eventScene.Update((float)dt);
        if (!fadingOut && eventScene.IsReadyToClose())
        {
            capturedEffect = eventScene.GetPendingEffect();
            fadingOut = true;
        }
    }

    if (fadingOut && fadeOverlay)
    {
        fadeAlpha += (float)dt / 600.0f;
        if (fadeAlpha >= 1.0f)
        {
            fadeAlpha = 1.0f;
            fadeOverlay->SetAlpha(fadeAlpha);
            eventScene.Close(objectsList);
            TransitionToLevel(capturedEffect);
            return;
        }
        fadeOverlay->SetAlpha(fadeAlpha);
    }
}

void EventPage::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void EventPage::LevelFree()
{
    eventScene.Close(objectsList);
    for (DrawableObject* obj : objectsList)
        delete obj;
    objectsList.clear();
    fadeOverlay = nullptr;
    fadingOut   = false;
    fadeAlpha   = 0.0f;
}

void EventPage::LevelUnload()
{
    GameEngine::GetInstance()->ClearMesh();
}

void EventPage::HandleKey(char key)
{
    if (key == 'q')
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
}

void EventPage::HandleMouse(int type, int x, int y)
{
    if (!eventScene.IsActive()) return;

    float rx, ry;
    GetRealMousePos(x, y, rx, ry);

    if (type == 0) {
        SoundManager::Get().Play(SoundManager::SFX::UI_CLICK);
        eventScene.HandleMouseClick(rx, ry);
    } else if (type == 3)
        eventScene.HandleMouseHover(rx, ry);
}
