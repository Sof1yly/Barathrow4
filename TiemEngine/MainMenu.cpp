#include "MainMenu.h"
#include "SquareMeshVbo.h"
#include "GameStateList.h"
#include <iostream>

static const float BTN_W = 280.0f;
static const float BTN_H = 60.0f;
static const float BTN_X = -400.0f;

void MainMenu::InitButton(MenuButton& btn, const std::string& text,
                           const std::string& texPath, SDL_Color labelColor,
                           glm::vec3 pos, glm::vec2 size)
{
    btn.pos  = pos;
    btn.size = size;

    btn.bg = new ImageObject();
    btn.bg->SetTexture(texPath);
    btn.bg->SetSize(size.x, -size.y);
    btn.bg->SetPosition(pos);
    objectsList.push_back(btn.bg);

    btn.label = new TextObject();
    btn.label->LoadText(text, labelColor, 30);
    btn.label->SetPosition(glm::vec3(pos.x, pos.y, pos.z + 1.0f));
    objectsList.push_back(btn.label);
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
    // Background
    auto* bg = new ImageObject();
    bg->SetSize(1920.0f, -1080.0f);
    bg->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    bg->SetTexture("../Resource/Texture/TestMainMenu.png");
    objectsList.push_back(bg);

    // Button textures
    const std::string whiteBtn = "../Resource/Texture/Mock/Whtg.png";
    const std::string redBtn   = "../Resource/Texture/Mock/Redg.png";

    // Label colors
    SDL_Color dark  = {40,  40,  40,  255};
    SDL_Color light = {255, 255, 255, 255};

    // Buttons stacked vertically on the left side
    InitButton(btnStart,    "Start Game",  whiteBtn, dark,  glm::vec3(BTN_X,  230.0f, 1.0f), glm::vec2(BTN_W, BTN_H));
    InitButton(btnContinue, "Continue",    whiteBtn, dark,  glm::vec3(BTN_X,  115.0f, 1.0f), glm::vec2(BTN_W, BTN_H));
    InitButton(btnAbandon,  "Abandon Run", whiteBtn, dark,  glm::vec3(BTN_X,    0.0f, 1.0f), glm::vec2(BTN_W, BTN_H));
    InitButton(btnSettings, "Settings",    whiteBtn, dark,  glm::vec3(BTN_X, -115.0f, 1.0f), glm::vec2(BTN_W, BTN_H));
    InitButton(btnQuit,     "Quit Game",   redBtn,   light, glm::vec3(BTN_X, -230.0f, 1.0f), glm::vec2(BTN_W, BTN_H));
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

    MenuButton* buttons[] = { &btnStart, &btnContinue, &btnAbandon, &btnSettings, &btnQuit };

    if (type == 3) // hover — highlight hovered button
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

    if (type != 0) return; // only handle left-click down from here

    if (btnStart.IsClicked(rx, ry))
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1;
    }
    else if (btnContinue.IsClicked(rx, ry))
    {
        // No save system yet — treat as start new game
        GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1;
    }
    else if (btnAbandon.IsClicked(rx, ry))
    {
        std::cout << "[MainMenu] No active run to abandon.\n";
    }
    else if (btnSettings.IsClicked(rx, ry))
    {
        std::cout << "[MainMenu] Settings not implemented yet.\n";
    }
    else if (btnQuit.IsClicked(rx, ry))
    {
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
    }
}
