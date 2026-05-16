#include "EventPage.h"
#include "SquareMeshVbo.h"
#include "GameStateList.h"
#include "GameData.h"
#include "SaveSystem.h"

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
}

void EventPage::LevelUpdate()
{
    int dt = GameEngine::GetInstance()->GetDeltaTime();

    for (auto* obj : objectsList)
        obj->Update((float)dt);

    if (eventScene.IsActive())
    {
        eventScene.Update((float)dt);
        if (eventScene.IsReadyToClose())
        {
            EventScene::EffectType effect = eventScene.GetPendingEffect();
            eventScene.Close(objectsList);
            TransitionToLevel(effect);
        }
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

    if (type == 0)
        eventScene.HandleMouseClick(rx, ry);
    else if (type == 3)
        eventScene.HandleMouseHover(rx, ry);
}
