#include "TutorialPage.h"
#include "SquareMeshVbo.h"
#include "GameStateList.h"
#include "GameData.h"
#include "SaveSystem.h"
#include <iostream>

void TutorialPage::GetRealMousePos(int x, int y, float& rx, float& ry) const
{
    int   winW   = GameEngine::GetInstance()->GetWindowWidth();
    int   winH   = GameEngine::GetInstance()->GetWindowHeight();
    float scaleW = GameEngine::GetInstance()->GetDrawAreaWidth();
    float scaleH = GameEngine::GetInstance()->GetDrawAreaHeight();
    rx = (x - winW  / 2.0f) * (scaleW / winW);
    ry = (winH / 2.0f - y)  * (scaleH / winH);
}

void TutorialPage::ShowPage(int index)
{
    for (int i = 0; i < PAGE_COUNT; i++)
        if (pages[i]) pages[i]->SetAlpha(i == index ? 1.0f : 0.0f);

    // Update hint: last page says "Click to start" instead of "Next"
    if (hintText)
    {
        SDL_Color c = { 220, 220, 220, 200 };
        if (index == PAGE_COUNT - 1)
            hintText->LoadText("Click right to start game  |  Click left to go back", c, 22);
        else if (index == 0)
            hintText->LoadText("Click right to continue", c, 22);
        else
            hintText->LoadText("Click left to go back  |  Click right to continue", c, 22);
    }
}

void TutorialPage::LevelLoad()
{
    auto* square = new SquareMeshVbo();
    square->LoadData();
    GameEngine::GetInstance()->AddMesh(SquareMeshVbo::MESH_NAME, square);
}

void TutorialPage::LevelInit()
{
    currentPage = 0;

    // Load all 5 tutorial pages (hidden by default, only page 0 shown)
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        std::string path = "../Resource/Texture/tutorial/" + std::to_string(i + 1) + ".png";
        pages[i] = new ImageObject();
        pages[i]->SetTexture(path);
        pages[i]->SetSize(1920.0f, -1080.0f);
        pages[i]->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
        pages[i]->SetAlpha(0.0f);
        objectsList.push_back(pages[i]);
    }

    // Navigation hint at bottom of screen
    hintText = new TextObject();
    SDL_Color c = { 220, 220, 220, 200 };
    hintText->LoadText("Click right to continue", c, 22);
    hintText->SetPosition(glm::vec3(0.0f, -490.0f, 2.0f));
    objectsList.push_back(hintText);

    ShowPage(currentPage);
}

void TutorialPage::LevelUpdate()
{
}

void TutorialPage::LevelDraw()
{
    GameEngine::GetInstance()->Render(objectsList);
}

void TutorialPage::LevelFree()
{
    for (DrawableObject* obj : objectsList)
        delete obj;
    objectsList.clear();

    for (int i = 0; i < PAGE_COUNT; i++)
        pages[i] = nullptr;
    hintText = nullptr;
}

void TutorialPage::LevelUnload()
{
    GameEngine::GetInstance()->ClearMesh();
}

void TutorialPage::HandleKey(char key)
{
    if (key == 'q')
        GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
}

void TutorialPage::HandleMouse(int type, int x, int y)
{
    if (type != 0) return;  // only handle left-button-down clicks

    float rx, ry;
    GetRealMousePos(x, y, rx, ry);

    if (rx >= 0.0f)
    {
        // Right half -> next page
        if (currentPage < PAGE_COUNT - 1)
        {
            currentPage++;
            ShowPage(currentPage);
        }
        else
        {
            // Last page, right click -> tutorial done, go straight to level (skip blessing)
            SaveSystem::MarkPlayed();
            GameData::GetInstance()->gGameStateNext = GameState::GS_LEVEL1;
        }
    }
    else
    {
        // Left half -> previous page (nothing on page 0)
        if (currentPage > 0)
        {
            currentPage--;
            ShowPage(currentPage);
        }
    }
}
