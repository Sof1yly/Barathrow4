#include "MainMenu.h"
#include "SquareMeshVbo.h"
#include "GameStateList.h"
#include "SaveSystem.h"
#include "SettingPage.h"
#include "SoundManager.h"
#include <iostream>

static const char* TEX_BG       = "../Resource/Texture/UI/Menu/BG.png";
static const char* TEX_FADE     = "../Resource/Texture/UI/Menu/Fade.png";
static const char* TEX_LOGO     = "../Resource/Texture/UI/Menu/Logo.png";
static const char* TEX_START    = "../Resource/Texture/UI/Menu/START GAME.png";
static const char* TEX_CONT     = "../Resource/Texture/UI/Menu/CONTINUE.png";
static const char* TEX_ABANDON  = "../Resource/Texture/UI/Menu/ABANDON RUN.png";
static const char* TEX_SETTINGS = "../Resource/Texture/UI/Menu/SETTINGS.png";
static const char* TEX_QUIT     = "../Resource/Texture/UI/Menu/QUIT GAME.png";

void MainMenu::InitButton(MenuButton& btn, const std::string& texPath,glm::vec3 pos, glm::vec2 size)
{
    btn.pos  = pos;
    btn.size = size;

    btn.bg = new ImageObject();
    btn.bg->SetTexture(texPath);
    btn.bg->SetSize(size.x, -size.y);
    btn.bg->SetPosition(pos);
    objectsList.push_back(btn.bg);

    btn.label = nullptr;
}

void MainMenu::GetRealMousePos(int x, int y, float& rx, float& ry) const
{
    int   winW   = GameEngine::GetInstance()->GetWindowWidth();
    int   winH   = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();

    rx = (x - winW / 2.0f) * (scaleW / winW);
    ry = (winH / 2.0f - y) * (scaleH / winH);
}

void MainMenu::LevelLoad()
{
    auto* square = new SquareMeshVbo();
    square->LoadData();
    GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);
}

void MainMenu::LevelInit()
{
    // 1. Background — full screen
    auto* bg = new ImageObject();
    bg->SetTexture(TEX_BG);
    bg->SetSize(1920.0f, -1080.0f);
    bg->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    objectsList.push_back(bg);

    // 2. Fade
    auto* fade = new ImageObject();
    fade->SetTexture(TEX_FADE);
    fade->SetSize(1920.0f, -1080.0f);
    fade->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
    objectsList.push_back(fade);

    // 3. Logo 
    auto* logo = new ImageObject();
    logo->SetTexture(TEX_LOGO);
    logo->SetSize(551.0f, -264.0f);
    logo->SetPosition(glm::vec3(-580.0f, 320.0f, 2.0f));
    objectsList.push_back(logo);

    // 4. Buttons 

    InitButton(btnStart,    TEX_START,    glm::vec3(-635.0f,  70.0f, 3.0f), glm::vec2(329.0f, 41.0f));
    InitButton(btnContinue, TEX_CONT,     glm::vec3(-675.0f, -16.0f, 3.0f), glm::vec2(248.0f, 41.0f));
    InitButton(btnAbandon,  TEX_ABANDON,  glm::vec3(-627.0f,-107.0f, 3.0f), glm::vec2(342.0f, 41.0f));
    InitButton(btnSettings, TEX_SETTINGS, glm::vec3(-699.0f,-255.0f, 3.0f), glm::vec2(193.5f, 29.0f));
    InitButton(btnQuit,     TEX_QUIT,     glm::vec3(-704.0f,-330.0f, 3.0f), glm::vec2(179.5f, 29.0f));

    settingPage.Init(objectsList);
}

void MainMenu::LevelUpdate()
{
}

void MainMenu::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void MainMenu::LevelFree()
{
    settingPage.Reset();   // null out pointers before objectsList deletes them
    for (DrawableObject* obj : objectsList)
        delete obj;
    objectsList.clear();

    btnStart    = MenuButton{};
    btnContinue = MenuButton{};
    btnAbandon  = MenuButton{};
    btnSettings = MenuButton{};
    btnQuit     = MenuButton{};
}

void MainMenu::LevelUnload()
{
    GameEngine::GetInstance()->ClearMesh();
}

void MainMenu::HandleKey(char key)
{
    if (key == 'q')
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
}

void MainMenu::HandleMouse(int type, int x, int y)
{
    float rx, ry;
    GetRealMousePos(x, y, rx, ry);

    // Route all input to the setting page while it is open
    if (settingPageActive)
    {
        if (type == 3)
            settingPage.HandleHover(rx, ry);
        else if (type == 0)
        {
            if (settingPage.HandleClick(rx, ry) == SettingPage::Action::CLOSE)
                settingPageActive = false;
        }
        return;
    }

    MenuButton* buttons[] = { &btnStart, &btnContinue, &btnAbandon, &btnSettings, &btnQuit };

    if (type == 3) // hover — dim non-hovered buttons
    {
        bool anyHovered = false;
        for (auto* btn : buttons)
        {
            if (btn->IsClicked(rx, ry)) {
                if (btn->bg) btn->bg->SetAlpha(1.0f);
                anyHovered = true;
            } else {
                if (btn->bg) btn->bg->SetAlpha(0.65f);
            }
        }
        if (!anyHovered)
        {
            for (auto* btn : buttons)
                if (btn->bg) btn->bg->SetAlpha(1.0f);
        }
        return;
    }

    if (type != 0) return;

    // Play UI click for any button press
    MenuButton* buttons2[] = { &btnStart, &btnContinue, &btnAbandon, &btnSettings, &btnQuit };
    for (auto* btn : buttons2)
        if (btn->IsClicked(rx, ry)) { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); break; }

    if (btnStart.IsClicked(rx, ry))
    {
        // First-time player: show tutorial and skip blessing.
        // Returning player: go straight to the blessing (event) page.
        if (!SaveSystem::HasPlayedBefore())
            GameData::GetInstance()->gGameStateNext = GameState::GS_TUTORIAL;
        else
            GameData::GetInstance()->gGameStateNext = GameState::GS_EVENT_PAGE;
    }
    else if (btnContinue.IsClicked(rx, ry))
    {
        if (SaveSystem::HasSaveFile()) {
            SaveSystem::pendingLoad = true;
            GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1;
        } else {
            std::cout << "[MainMenu] No save file found.\n";
        }
    }
    else if (btnAbandon.IsClicked(rx, ry))
    {
        SaveSystem::DeleteSave();
        std::cout << "[MainMenu] Run abandoned. Save deleted.\n";
    }
    else if (btnSettings.IsClicked(rx, ry))
    {
        settingPage.Show(objectsList);
        settingPageActive = true;
    }
    else if (btnQuit.IsClicked(rx, ry))
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
    }
}
