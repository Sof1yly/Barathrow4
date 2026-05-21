#include "PauseMenu.h"
#include "Button.h"
#include "SoundManager.h"
#include <SDL.h>
#include <algorithm>

static const std::string DIR = "../Resource/Texture/UI/PauseMenu/";

// Button dimensions — wider to accommodate long labels
static const float BTN_W = 620.0f;
static const float BTN_H = 122.0f;
// Step between button centres (height + gap)
static const float BTN_STEP = BTN_H + 15.0f;

// Z layers — sit above cards (max z=1000) and match SettingPage hierarchy
static const float OVL_Z = 1100.0f;
static const float BTN_Z = 1102.0f;
static const float TXT_Z = 1103.0f;

const glm::vec3 PauseMenu::HIDDEN = glm::vec3(0.0f, 10000.0f, 0.0f);

// ---------------------------------------------------------------------------
bool PauseMenu::PauseButton::IsClicked(float wx, float wy) const {
    if (!enabled) return false;
    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;
    return wx >= pos.x - hw && wx <= pos.x + hw &&
           wy >= pos.y - hh && wy <= pos.y + hh;
}

// ---------------------------------------------------------------------------
void PauseMenu::AddPauseObject(DrawableObject* obj, glm::vec3 visiblePos,
                                std::vector<DrawableObject*>& objectsList)
{
    obj->SetPosition(HIDDEN);
    objectsList.push_back(obj);
    pauseObjects.push_back({ obj, visiblePos });
}

void PauseMenu::AddGameOverObject(DrawableObject* obj, glm::vec3 visiblePos,
                                   std::vector<DrawableObject*>& objectsList)
{
    obj->SetPosition(HIDDEN);
    objectsList.push_back(obj);
    gameOverObjects.push_back({ obj, visiblePos });
}

void PauseMenu::InitButton(PauseButton& btn, const std::string& labelStr,
    SDL_Color color,
    glm::vec3 pos, glm::vec2 size,
    std::vector<DrawableObject*>& objectsList)
{
    btn.pos     = pos;
    btn.size    = size;
    btn.enabled = true;

    btn.bg = new ImageObject();
    btn.bg->SetTexture(DIR + "button_icon.png");
    btn.bg->SetSize(size.x, -size.y);
    btn.bg->SetAlpha(1.0f);
    AddPauseObject(btn.bg, glm::vec3(pos.x, pos.y, BTN_Z), objectsList);

    btn.label = new TextObject();
    btn.label->LoadText(labelStr, color, 34);
    AddPauseObject(btn.label, glm::vec3(pos.x, pos.y, TXT_Z), objectsList);
}

// ---------------------------------------------------------------------------
void PauseMenu::Init(std::vector<DrawableObject*>& objectsList)
{
    SDL_Color white = { 240, 240, 240, 255 };
    SDL_Color title = { 255, 255, 255, 255 };

    // Full-screen dim overlay
    panel = new ImageObject();
    panel->SetColor(0.0f, 0.0f, 0.0f);
    panel->SetAlpha(0.65f);
    panel->SetSize(1920.0f, -1080.0f);
    AddPauseObject(panel, glm::vec3(0.0f, 0.0f, OVL_Z), objectsList);

    // "PAUSE" title
    titleText = new TextObject();
    titleText->LoadText("PAUSE", title, 64);
    AddPauseObject(titleText, glm::vec3(0.0f, 330.0f, TXT_Z), objectsList);

    // 5 buttons top to bottom, centred on x=0
    //   Resume             y = +175
    //   Setting            y = +175 - BTN_STEP
    //   Abandon Run        y = +175 - 2*BTN_STEP
    //   Save & Quit Main   y = +175 - 3*BTN_STEP
    //   Save & Quit Desk   y = +175 - 4*BTN_STEP
    const float TOP_Y = 175.0f;
    InitButton(btnResume,      "Resume",                   white, glm::vec3(0.0f, TOP_Y - 0 * BTN_STEP, 0.0f), glm::vec2(BTN_W, BTN_H), objectsList);
    InitButton(btnSetting,     "Setting",                  white, glm::vec3(0.0f, TOP_Y - 1 * BTN_STEP, 0.0f), glm::vec2(BTN_W, BTN_H), objectsList);
    InitButton(btnAbandon,     "Abandon Run",              white, glm::vec3(0.0f, TOP_Y - 2 * BTN_STEP, 0.0f), glm::vec2(BTN_W, BTN_H), objectsList);
    InitButton(btnSaveQuit,    "Save & Quit to Main Menu", white, glm::vec3(0.0f, TOP_Y - 3 * BTN_STEP, 0.0f), glm::vec2(BTN_W, BTN_H), objectsList);
    InitButton(btnQuitDesktop, "Save & Quit to Desktop",   white, glm::vec3(0.0f, TOP_Y - 4 * BTN_STEP, 0.0f), glm::vec2(BTN_W, BTN_H), objectsList);

    visible = false;
}

void PauseMenu::Show(std::vector<DrawableObject*>& objectsList)
{
    visible = true;
    Button::setMenu(true);
    // Re-append every pause object to the END of objectsList so they always
    // render on top, regardless of which card was last hovered.
    for (auto& pair : pauseObjects) {
        if (!pair.first) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), pair.first);
        if (it != objectsList.end()) objectsList.erase(it);
        objectsList.push_back(pair.first);
        pair.first->SetPosition(pair.second);
    }
}

void PauseMenu::Hide()
{
    visible = false;
    Button::setMenu(false);
    for (auto& pair : pauseObjects)
        if (pair.first) pair.first->SetPosition(HIDDEN);

    PauseButton* buttons[] = { &btnResume, &btnSetting, &btnAbandon, &btnSaveQuit, &btnQuitDesktop };
    for (auto* btn : buttons) {
        if (btn->bg) {
            btn->bg->SetColor(1.0f, 1.0f, 1.0f);
            btn->bg->SetAlpha(1.0f);
        }
    }
}

void PauseMenu::Reset()
{
    panel          = nullptr;
    titleText      = nullptr;
    btnResume      = PauseButton{};
    btnSetting     = PauseButton{};
    btnAbandon     = PauseButton{};
    btnSaveQuit    = PauseButton{};
    btnQuitDesktop = PauseButton{};
    pauseObjects.clear();
    visible = false;

    goPanel        = nullptr;
    goTitleText    = nullptr;
    btnRetry       = PauseButton{};
    btnGoMain      = PauseButton{};
    gameOverObjects.clear();
    gameOverVisible = false;
}

// ---------------------------------------------------------------------------
void PauseMenu::InitGameOver(std::vector<DrawableObject*>& objectsList)
{
    SDL_Color white = { 240, 240, 240, 255 };
    SDL_Color red   = { 220,  60,  60, 255 };

    // Full-screen dim overlay
    goPanel = new ImageObject();
    goPanel->SetColor(0.0f, 0.0f, 0.0f);
    goPanel->SetAlpha(0.75f);
    goPanel->SetSize(1920.0f, -1080.0f);
    AddGameOverObject(goPanel, glm::vec3(0.0f, 0.0f, OVL_Z), objectsList);

    // "GAME OVER" title
    goTitleText = new TextObject();
    goTitleText->LoadText("GAME OVER", red, 80);
    AddGameOverObject(goTitleText, glm::vec3(0.0f, 200.0f, TXT_Z), objectsList);

    // "Retry" button
    const float TOP_Y  = 30.0f;
    const float MAIN_Y = TOP_Y - BTN_STEP;

    btnRetry.pos     = glm::vec3(0.0f, TOP_Y, 0.0f);
    btnRetry.size    = glm::vec2(BTN_W, BTN_H);
    btnRetry.enabled = true;
    btnRetry.bg = new ImageObject();
    btnRetry.bg->SetTexture(DIR + "button_icon.png");
    btnRetry.bg->SetSize(BTN_W, -BTN_H);
    AddGameOverObject(btnRetry.bg, glm::vec3(0.0f, TOP_Y, BTN_Z), objectsList);
    btnRetry.label = new TextObject();
    btnRetry.label->LoadText("Retry", white, 34);
    AddGameOverObject(btnRetry.label, glm::vec3(0.0f, TOP_Y, TXT_Z), objectsList);

    // "Back to Main Menu" button
    btnGoMain.pos     = glm::vec3(0.0f, MAIN_Y, 0.0f);
    btnGoMain.size    = glm::vec2(BTN_W, BTN_H);
    btnGoMain.enabled = true;
    btnGoMain.bg = new ImageObject();
    btnGoMain.bg->SetTexture(DIR + "button_icon.png");
    btnGoMain.bg->SetSize(BTN_W, -BTN_H);
    AddGameOverObject(btnGoMain.bg, glm::vec3(0.0f, MAIN_Y, BTN_Z), objectsList);
    btnGoMain.label = new TextObject();
    btnGoMain.label->LoadText("Back to Main Menu", white, 34);
    AddGameOverObject(btnGoMain.label, glm::vec3(0.0f, MAIN_Y, TXT_Z), objectsList);

    gameOverVisible = false;
}

void PauseMenu::ShowGameOver(std::vector<DrawableObject*>& objectsList)
{
    gameOverVisible = true;
    for (auto& pair : gameOverObjects) {
        if (!pair.first) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), pair.first);
        if (it != objectsList.end()) objectsList.erase(it);
        objectsList.push_back(pair.first);
        pair.first->SetPosition(pair.second);
    }
}

void PauseMenu::HideGameOver()
{
    gameOverVisible = false;
    for (auto& pair : gameOverObjects)
        if (pair.first) pair.first->SetPosition(HIDDEN);

    if (btnRetry.bg)  { btnRetry.bg->SetColor(1.0f, 1.0f, 1.0f);  btnRetry.bg->SetAlpha(1.0f); }
    if (btnGoMain.bg) { btnGoMain.bg->SetColor(1.0f, 1.0f, 1.0f); btnGoMain.bg->SetAlpha(1.0f); }
}

// ---------------------------------------------------------------------------
PauseMenu::Action PauseMenu::HandleClick(float wx, float wy)
{
    if (gameOverVisible) {
        if (btnRetry.IsClicked(wx, wy))  { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::RETRY; }
        if (btnGoMain.IsClicked(wx, wy)) { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::GAME_OVER_MAIN; }
        return Action::NONE;
    }
    if (!visible) return Action::NONE;
    if (btnResume.IsClicked(wx, wy))      { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::RESUME; }
    if (btnSetting.IsClicked(wx, wy))     { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::SETTING; }
    if (btnAbandon.IsClicked(wx, wy))     { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::ABANDON; }
    if (btnSaveQuit.IsClicked(wx, wy))    { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::SAVE_QUIT_MAIN; }
    if (btnQuitDesktop.IsClicked(wx, wy)) { SoundManager::Get().Play(SoundManager::SFX::UI_CLICK); return Action::SAVE_QUIT_DESKTOP; }
    return Action::NONE;
}

void PauseMenu::HandleHover(float wx, float wy)
{
    if (gameOverVisible) {
        PauseButton* btns[] = { &btnRetry, &btnGoMain };
        for (auto* btn : btns) {
            if (!btn->bg) continue;
            if (btn->IsClicked(wx, wy)) {
                btn->bg->SetAlpha(1.0f);
                btn->bg->SetColor(4.0f, 3.0f, 1.5f);
            } else {
                btn->bg->SetAlpha(1.0f);
                btn->bg->SetColor(1.0f, 1.0f, 1.0f);
            }
        }
        return;
    }
    if (!visible) return;
    PauseButton* buttons[] = { &btnResume, &btnSetting, &btnAbandon, &btnSaveQuit, &btnQuitDesktop };
    for (auto* btn : buttons) {
        if (!btn->bg) continue;
        if (btn->IsClicked(wx, wy)) {
            // Overbright warm gold — multiplies the dark texture pixels visibly lighter
            btn->bg->SetAlpha(1.0f);
            btn->bg->SetColor(4.0f, 3.0f, 1.5f);
        } else {
            btn->bg->SetAlpha(1.0f);
            btn->bg->SetColor(1.0f, 1.0f, 1.0f);
        }
    }
}
