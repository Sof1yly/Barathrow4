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
static const char* TEX_CREDITS  = "../Resource/Texture/UI/Menu/CREDITS.png";
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

    InitButton(btnCredits, TEX_CREDITS, glm::vec3(-699.0f, -181.0f, 3.0f), glm::vec2(193.5f, 29.0f));

    // Credits overlay — black full-screen cover + text lines (hidden initially)
    {
        creditsOverlay = new ImageObject();
        creditsOverlay->SetTexture(TEX_BG);
        creditsOverlay->SetColor(0.0f, 0.0f, 0.0f);
        creditsOverlay->SetSize(1920.0f, -1080.0f);
        creditsOverlay->SetPosition(glm::vec3(0.0f, 0.0f, 8.0f));
        creditsOverlay->SetAlpha(0.0f);
        objectsList.push_back(creditsOverlay);

        SDL_Color white = { 255, 255, 255, 255 };

        auto addLine = [&](const char* text, float y, int fontSize) {
            auto* t = new TextObject();
            t->LoadText(text, white, fontSize);
            t->SetPosition(glm::vec3(0.0f, y, 9.0f));
            t->SetAlpha(0.0f);
            creditTexts.push_back(t);
            objectsList.push_back(t);
        };

        addLine("- CREDITS -",          500.0f, 36);
        addLine("Game Designer & Game Director - Thanpat Pathomkasikul",           400.0f, 28);
        addLine("Producer & Programmer - Pupa Thapanapongpaiboon",            350.0f, 28);
        addLine("Lead Programmer  - Puri Ngamrumyong",             300.0f, 28);
        addLine("Art Director - Nichapa Boonjaiyai",           250.0f, 28);
        addLine("Concept & Environment Artist - Natthaya Deepichan",          200.0f, 28);
        addLine("Concept & Character Artist - Chayapol Chotivanich",          150.0f, 28);
        addLine("- Sound -", 50.0f, 36);
        addLine("BGM - Cyberpunk Synthwave - Alan Souls", 0.0f, 28);
        addLine("SFX - Pixabay - Various Artist", -50.0f, 28);
		addLine("The Game is made as an in-class project for DD290 Digital Design Project and DT230 Advance Project.", -200.0f, 28);
		addLine(" The project in its entirety belongs to ©COPYRIGHT 2026 DIGITAL DESIGN AND CREATIVE TECHNOLOGY. All Rights Reserved.", -250.0f, 28);
        addLine("Press any key to return", -360.0f, 22);
    }

    settingPage.Init(objectsList);

    // Fade overlay — drawn on top of everything; starts invisible
    fadeOverlay = new ImageObject();
    fadeOverlay->SetTexture(TEX_BG);
    fadeOverlay->SetColor(0.0f, 0.0f, 0.0f);
    fadeOverlay->SetSize(1920.0f, -1080.0f);
    fadeOverlay->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    fadeOverlay->SetAlpha(0.0f);
    objectsList.push_back(fadeOverlay);
}

void MainMenu::LevelUpdate()
{
    if (!fadeOut || !fadeOverlay) return;

    int dt = GameEngine::GetInstance()->GetDeltaTime();
    fadeAlpha += (float)dt / 600.0f;
    if (fadeAlpha >= 1.0f)
    {
        fadeAlpha = 1.0f;
        GameData::GetInstance()->gGameStateNext = pendingState;
    }
    fadeOverlay->SetAlpha(fadeAlpha);
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
    btnCredits  = MenuButton{};
    btnSettings = MenuButton{};
    btnQuit     = MenuButton{};
    creditsActive  = false;
    creditsOverlay = nullptr;
    creditTexts.clear();
    fadeOverlay = nullptr;
    fadeOut     = false;
    fadeAlpha   = 0.0f;
}

void MainMenu::LevelUnload()
{
    GameEngine::GetInstance()->ClearMesh();
}

void MainMenu::HandleKey(char key)
{
    if (creditsActive)
    {
        creditsActive = false;
        if (creditsOverlay) creditsOverlay->SetAlpha(0.0f);
        for (auto* t : creditTexts) t->SetAlpha(0.0f);
        return;
    }
    if (key == 'q')
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
}

void MainMenu::HandleMouse(int type, int x, int y)
{
    if (fadeOut) return;

    float rx, ry;
    GetRealMousePos(x, y, rx, ry);

    // Block all input while credits screen is shown (any click also dismisses it)
    if (creditsActive)
    {
        if (type == 0)
        {
            creditsActive = false;
            if (creditsOverlay) creditsOverlay->SetAlpha(0.0f);
            for (auto* t : creditTexts) t->SetAlpha(0.0f);
        }
        return;
    }

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

    MenuButton* buttons[] = { &btnStart, &btnContinue, &btnAbandon, &btnCredits, &btnSettings, &btnQuit };

    if (type == 3) // hover — dim non-hovered buttons
    {
        bool anyHovered = false;
        for (auto* btn : buttons)
        {
            bool hov = btn->IsClicked(rx, ry);
            if (hov) anyHovered = true;
            float a = hov ? 1.0f : 0.65f;
            if (btn->bg)    btn->bg->SetAlpha(a);
            if (btn->label) btn->label->SetAlpha(a);
        }
        if (!anyHovered)
        {
            for (auto* btn : buttons)
            {
                if (btn->bg)    btn->bg->SetAlpha(1.0f);
                if (btn->label) btn->label->SetAlpha(1.0f);
            }
        }
        return;
    }

    if (type != 0) return;

    // Play UI click for any button press
    for (auto* btn : buttons)
        if (btn->IsClicked(rx, ry)) { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); break; }

    if (btnStart.IsClicked(rx, ry))
    {
        fadeOut = true;
        fadeAlpha = 0.0f;
        pendingState = SaveSystem::HasPlayedBefore() ? GameState::GS_EVENT_PAGE : GameState::GS_TUTORIAL;
    }
    else if (btnContinue.IsClicked(rx, ry))
    {
        if (SaveSystem::HasSaveFile()) {
            SaveSystem::pendingLoad = true;
            fadeOut = true;
            fadeAlpha = 0.0f;
            pendingState = GameState::GS_LEVEL1;
        } else {
            std::cout << "[MainMenu] No save file found.\n";
        }
    }
    else if (btnAbandon.IsClicked(rx, ry))
    {
        SaveSystem::DeleteSave();
        std::cout << "[MainMenu] Run abandoned. Save deleted.\n";
    }
    else if (btnCredits.IsClicked(rx, ry))
    {
        creditsActive = true;
        if (creditsOverlay) creditsOverlay->SetAlpha(1.0f);
        for (auto* t : creditTexts) t->SetAlpha(1.0f);
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
