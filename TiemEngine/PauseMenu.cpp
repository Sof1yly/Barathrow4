#include "PauseMenu.h"
#include "Button.h"
#include <algorithm>

static const float BTN_W = 450.0f;
static const float BTN_H = 70.0f;
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

void PauseMenu::InitButton(PauseButton& btn, const std::string& labelStr,
    const std::string& texPath, SDL_Color color,
    glm::vec3 pos, glm::vec2 size, bool enabled,
    std::vector<DrawableObject*>& objectsList)
{
    btn.pos     = pos;
    btn.size    = size;
    btn.enabled = enabled;

    btn.bg = new ImageObject();
    btn.bg->SetTexture(texPath);
    btn.bg->SetSize(size.x, -size.y);
    if (!enabled) btn.bg->SetAlpha(0.35f);
    AddPauseObject(btn.bg, pos, objectsList);

    btn.label = new TextObject();
    btn.label->LoadText(labelStr, color, 32);
    if (!enabled) btn.label->SetAlpha(0.5f);
    AddPauseObject(btn.label, glm::vec3(pos.x, pos.y, pos.z + 1.0f), objectsList);
}

// ---------------------------------------------------------------------------
void PauseMenu::Init(std::vector<DrawableObject*>& objectsList)
{
    const std::string whiteBtn = "../Resource/Texture/Mock/Whtg.png";
    const std::string redBtn   = "../Resource/Texture/Mock/Redg.png";
    SDL_Color dark  = { 40,  40,  40, 255 };
    SDL_Color light = { 255, 255, 255, 255 };
    SDL_Color gray  = { 120, 120, 120, 255 };

    // Full-screen black overlay (1920x1080 world units)
    panel = new ImageObject();
    panel->SetColor(0.0f, 0.0f, 0.0f);
    panel->SetAlpha(0.6f);
    panel->SetSize(1920.0f, -1080.0f);
    AddPauseObject(panel, glm::vec3(0.0f, 0.0f, 87.0f), objectsList);

    // Title
    titleText = new TextObject();
    SDL_Color titleColor = { 255, 230, 100, 255 };
    titleText->LoadText("PAUSE", titleColor, 64);
    AddPauseObject(titleText, glm::vec3(0.0f, 240.0f, 90.0f), objectsList);

    // Buttons (top to bottom, 82 px spacing, 12 px gap between 70 px tall buttons)
    //   y= 150: Resume
    //   y=  68: Setting            (disabled)
    //   y= -14: Abandon Run
    //   y= -96: Save & Quit to Main
    //   y=-178: Save & Quit to Desktop (disabled)
    InitButton(btnResume,      "Resume",                  whiteBtn, dark,
               glm::vec3(0.0f,  150.0f, 89.0f), glm::vec2(BTN_W, BTN_H), true,  objectsList);
    InitButton(btnSetting,     "Setting",                 whiteBtn, gray,
               glm::vec3(0.0f,   68.0f, 89.0f), glm::vec2(BTN_W, BTN_H), false, objectsList);
    InitButton(btnAbandon,     "Abandon Run",             redBtn,   light,
               glm::vec3(0.0f,  -14.0f, 89.0f), glm::vec2(BTN_W, BTN_H), true,  objectsList);
    InitButton(btnSaveQuit,    "Save & Quit to Main",     whiteBtn, dark,
               glm::vec3(0.0f,  -96.0f, 89.0f), glm::vec2(BTN_W, BTN_H), true,  objectsList);
    InitButton(btnQuitDesktop, "Save & Quit to Desktop",  whiteBtn, gray,
               glm::vec3(0.0f, -178.0f, 89.0f), glm::vec2(BTN_W, BTN_H), false, objectsList);

    visible = false;
}

void PauseMenu::Show()
{
    visible = true;
    Button::setMenu(true);
    for (auto& pair : pauseObjects)
        if (pair.first) pair.first->SetPosition(pair.second);
}

void PauseMenu::Hide()
{
    visible = false;
    Button::setMenu(false);
    for (auto& pair : pauseObjects)
        if (pair.first) pair.first->SetPosition(HIDDEN);
}

void PauseMenu::Reset()
{
    panel        = nullptr;
    titleText    = nullptr;
    btnResume      = PauseButton{};
    btnSetting     = PauseButton{};
    btnAbandon     = PauseButton{};
    btnSaveQuit    = PauseButton{};
    btnQuitDesktop = PauseButton{};
    pauseObjects.clear();
    visible = false;
}

// ---------------------------------------------------------------------------
PauseMenu::Action PauseMenu::HandleClick(float wx, float wy)
{
    if (!visible) return Action::NONE;
    if (btnResume.IsClicked(wx, wy))      return Action::RESUME;
    if (btnSaveQuit.IsClicked(wx, wy))    return Action::SAVE_QUIT_MAIN;
    if (btnQuitDesktop.IsClicked(wx, wy)) return Action::SAVE_QUIT_DESKTOP;
    if (btnAbandon.IsClicked(wx, wy))     return Action::ABANDON;
    return Action::NONE;
}

void PauseMenu::HandleHover(float wx, float wy)
{
    if (!visible) return;
    PauseButton* buttons[] = { &btnResume, &btnSetting, &btnAbandon, &btnSaveQuit, &btnQuitDesktop };
    for (auto* btn : buttons) {
        if (!btn->enabled) {
            // keep disabled buttons dimmed
            if (btn->bg) btn->bg->SetAlpha(0.35f);
            continue;
        }
        if (btn->IsClicked(wx, wy)) {
            if (btn->bg) btn->bg->SetAlpha(1.0f);
        } else {
            if (btn->bg) btn->bg->SetAlpha(0.65f);
        }
    }
}
