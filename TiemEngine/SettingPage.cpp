#include "SettingPage.h"
#include "GameData.h"
#include "GameEngine.h"
#include "GLRenderer.h"
#include <SDL.h>
#include <SDL_mixer.h>

// SDL window declared in Main.cpp
extern SDL_Window* window;

// ─────────────────────────────────────────────────────────────────────────────
// Layout constants (world units, 1920x1080 virtual space)
// ─────────────────────────────────────────────────────────────────────────────
namespace {
    // Cards in hand use z=100–190, dragged cards z=600, hovered cards z=1000.
    // Setting page must sit above all of those.
    constexpr float OVL_Z  = 1100.0f;
    constexpr float PNL_Z  = 1101.0f;
    constexpr float ELM_Z  = 1102.0f;
    constexpr float TOP_Z  = 1103.0f;

    // Row vertical centres
    constexpr float ROW1_Y =  165.0f;   // Music
    constexpr float ROW2_Y =   20.0f;   // Sound
    constexpr float ROW3_Y = -125.0f;   // Resolution
    constexpr float BTN_Y  = -285.0f;   // Apply / Back

    // ── Left column: text labels ─────────────────────────────────────────────
    // Pushed in from the backboard edge so text stays inside the panel.
    constexpr float LBLX   = -390.0f;   // centre of label text

    // ── Right column: interactive elements ──────────────────────────────────
    // Shifted right relative to the previous version so icons / bar clear
    // the label text with comfortable margin.
    constexpr float TOGX   = -185.0f;   // toggle icon (music/sound on-off)
    constexpr float LARRX  =  -80.0f;   // left arrow
    constexpr float BARX   =  160.0f;   // bar / resolution-text centre
    constexpr float RARRX  =  400.0f;   // right arrow

    // Element sizes
    constexpr float TOGSZ  =  75.0f;    // toggle icon (square)
    constexpr float ARRSZ  =  80.0f;    // arrow button (square)
    constexpr float BARW   = 384.0f;
    constexpr float BARH   = 109.0f;

    // Pieces: rendered narrower than the texture so equal gaps are visible.
    //   PCEW_D  = displayed width of each piece (< texture width 37)
    //   PCE_STP = centre-to-centre step between adjacent pieces
    //   gap between pieces = PCE_STP - PCEW_D = 9 world units
    constexpr float PCEW_D  =  28.0f;
    constexpr float PCE_STP =  37.0f;
    constexpr float PCEH    =  95.0f;
    // Centre of piece[0]: group centred inside the bar
    constexpr float PCE0X   = BARX - 4.5f * PCE_STP;   // = 160 - 166.5 = -6.5

    constexpr float RESSZ   =  90.0f;   // resolution icon width
    constexpr float RESH    =  70.0f;   // resolution icon height

    // Back / Apply buttons
    constexpr float BACKX   = -160.0f;
    constexpr float APPLYX  =  160.0f;
    constexpr float BTNBW   =  280.0f;
    constexpr float BTNBH   =   80.0f;

    const std::string DIR = "../Resource/Texture/UI/SettingUI/";
}

// ─────────────────────────────────────────────────────────────────────────────
const glm::vec3 SettingPage::HIDDEN = glm::vec3(0.0f, 10000.0f, 0.0f);

const SettingPage::ResOption SettingPage::RESOLUTIONS[3] = {
    { 1920, 1080, "1920 x 1080" },
    { 1600,  900, "1600 x 900"  },
    { 1280,  720, "1280 x 720"  },
};

// ─────────────────────────────────────────────────────────────────────────────
void SettingPage::AddObj(DrawableObject* obj, glm::vec3 visPos, std::vector<DrawableObject*>& lst) {
    obj->SetPosition(HIDDEN);
    lst.push_back(obj);
    settingObjects.push_back({ obj, visPos });
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingPage::Init(std::vector<DrawableObject*>& objectsList) {
    // Load persisted state
    auto* gd        = GameData::GetInstance();
    musicVolume     = gd->musicVolume;
    musicEnabled    = gd->musicEnabled;
    soundVolume     = gd->soundVolume;
    soundEnabled    = gd->soundEnabled;
    resolutionIndex = gd->resolutionIndex;

    SDL_Color labelCol = { 240, 240, 240, 255 };

    // ── Dim overlay ──────────────────────────────────────────────────────────
    overlay = new ImageObject();
    overlay->SetColor(0.0f, 0.0f, 0.0f);
    overlay->SetAlpha(0.65f);
    overlay->SetSize(1920.0f, -1080.0f);
    AddObj(overlay, glm::vec3(0.0f, 0.0f, OVL_Z), objectsList);

    // ── Backboard ────────────────────────────────────────────────────────────
    backboard = new ImageObject();
    backboard->SetTexture(DIR + "Backboard.png");
    backboard->SetSize(1121.0f, -748.0f);
    AddObj(backboard, glm::vec3(0.0f, 0.0f, PNL_Z), objectsList);

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel = new TextObject();
    SDL_Color titleCol = { 240, 240, 240, 255 };
    titleLabel->LoadText("SETTING", titleCol, 65);
    AddObj(titleLabel, glm::vec3(0.0f, 290.0f, TOP_Z), objectsList);

    // ── Row labels (left side) ────────────────────────────────────────────────
    musicLabel = new TextObject();
    musicLabel->LoadText("Music", labelCol, 28);
    AddObj(musicLabel, glm::vec3(LBLX, ROW1_Y, TOP_Z), objectsList);

    soundLabel = new TextObject();
    soundLabel->LoadText("Sound", labelCol, 28);
    AddObj(soundLabel, glm::vec3(LBLX, ROW2_Y, TOP_Z), objectsList);

    resLabel = new TextObject();
    resLabel->LoadText("Screen Resolution", labelCol, 22);
    AddObj(resLabel, glm::vec3(LBLX, ROW3_Y, TOP_Z), objectsList);

    // ── Helper: build music / sound row (identical structure) ────────────────
    auto buildRow = [&](float rowY,
                        ImageObject*& icon, const std::string& openTex,
                        ImageObject*& barBg,
                        ImageObject* pieces[10],
                        ImageObject*& leftArr, ImageObject*& rightArr,
                        Area& toggleArea, Area& leftArea, Area& rightArea)
    {
        // Toggle icon
        icon = new ImageObject();
        icon->SetTexture(openTex);
        icon->SetSize(TOGSZ, -TOGSZ);
        AddObj(icon, glm::vec3(TOGX, rowY, ELM_Z), objectsList);
        toggleArea = { TOGX, rowY, TOGSZ * 0.5f, TOGSZ * 0.5f };

        // Left arrow
        leftArr = new ImageObject();
        leftArr->SetTexture(DIR + "button left.png");
        leftArr->SetSize(ARRSZ, -ARRSZ);
        AddObj(leftArr, glm::vec3(LARRX, rowY, ELM_Z), objectsList);
        leftArea = { LARRX, rowY, ARRSZ * 0.5f, ARRSZ * 0.5f };

        // Bar background
        barBg = new ImageObject();
        barBg->SetTexture(DIR + "Bar.PNG");
        barBg->SetSize(BARW, -BARH);
        AddObj(barBg, glm::vec3(BARX, rowY, ELM_Z), objectsList);

        // 10 piece segments – rendered narrower than texture for equal visible gaps
        for (int i = 0; i < 10; i++) {
            pieces[i] = new ImageObject();
            pieces[i]->SetTexture(DIR + "Pieces.png");
            pieces[i]->SetSize(PCEW_D, -PCEH);
            float px = PCE0X + i * PCE_STP;
            AddObj(pieces[i], glm::vec3(px, rowY, TOP_Z), objectsList);
        }

        // Right arrow
        rightArr = new ImageObject();
        rightArr->SetTexture(DIR + "button right.png");
        rightArr->SetSize(ARRSZ, -ARRSZ);
        AddObj(rightArr, glm::vec3(RARRX, rowY, ELM_Z), objectsList);
        rightArea = { RARRX, rowY, ARRSZ * 0.5f, ARRSZ * 0.5f };
    };

    buildRow(ROW1_Y, musicIcon, DIR + "open_music.png",
             musicBarBg, musicPieces,
             musicLeftArrow, musicRightArrow,
             areaMusicToggle, areaMusicLeft, areaMusicRight);

    buildRow(ROW2_Y, soundIcon, DIR + "open_sound.png",
             soundBarBg, soundPieces,
             soundLeftArrow, soundRightArrow,
             areaSoundToggle, areaSoundLeft, areaSoundRight);

    // ── Resolution row ───────────────────────────────────────────────────────
    resIcon = new ImageObject();
    resIcon->SetTexture(DIR + "ScreenResolutionIcon.png");
    resIcon->SetSize(RESSZ, -RESH);
    AddObj(resIcon, glm::vec3(TOGX, ROW3_Y, ELM_Z), objectsList);

    resLeftArrow = new ImageObject();
    resLeftArrow->SetTexture(DIR + "button left.png");
    resLeftArrow->SetSize(ARRSZ, -ARRSZ);
    AddObj(resLeftArrow, glm::vec3(LARRX, ROW3_Y, ELM_Z), objectsList);
    areaResLeft = { LARRX, ROW3_Y, ARRSZ * 0.5f, ARRSZ * 0.5f };

    resText = new TextObject();
    resText->LoadText(RESOLUTIONS[resolutionIndex].label, labelCol, 30);
    AddObj(resText, glm::vec3(BARX, ROW3_Y, TOP_Z), objectsList);

    resRightArrow = new ImageObject();
    resRightArrow->SetTexture(DIR + "button right.png");
    resRightArrow->SetSize(ARRSZ, -ARRSZ);
    AddObj(resRightArrow, glm::vec3(RARRX, ROW3_Y, ELM_Z), objectsList);
    areaResRight = { RARRX, ROW3_Y, ARRSZ * 0.5f, ARRSZ * 0.5f };

    // ── Back / Apply buttons ─────────────────────────────────────────────────
    backBtnImg = new ImageObject();
    backBtnImg->SetTexture(DIR + "Back.png");
    backBtnImg->SetSize(BTNBW, -BTNBH);
    AddObj(backBtnImg, glm::vec3(BACKX, BTN_Y, ELM_Z), objectsList);
    areaBack = { BACKX, BTN_Y, BTNBW * 0.5f, BTNBH * 0.5f };

    applyBtnImg = new ImageObject();
    applyBtnImg->SetTexture(DIR + "Apply.png");
    applyBtnImg->SetSize(BTNBW, -BTNBH);
    AddObj(applyBtnImg, glm::vec3(APPLYX, BTN_Y, ELM_Z), objectsList);
    areaApply = { APPLYX, BTN_Y, BTNBW * 0.5f, BTNBH * 0.5f };

    // Button text labels drawn on top of the button images
    SDL_Color darkCol  = {  40,  40,  40, 255 };
    SDL_Color lightCol = { 240, 240, 240, 255 };

    backLabel = new TextObject();
    backLabel->LoadText("Back", darkCol, 32);
    AddObj(backLabel, glm::vec3(BACKX, BTN_Y, TOP_Z), objectsList);

    applyLabel = new TextObject();
    applyLabel->LoadText("Apply", lightCol, 32);
    AddObj(applyLabel, glm::vec3(APPLYX, BTN_Y, TOP_Z), objectsList);

    visible = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingPage::Show(std::vector<DrawableObject*>& objectsList) {
    visible = true;
    for (auto& p : settingObjects) {
        if (!p.first) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), p.first);
        if (it != objectsList.end()) objectsList.erase(it);
        objectsList.push_back(p.first);
        p.first->SetPosition(p.second);
    }
    UpdateMusicIcon();
    UpdateSoundIcon();
    UpdateMusicPieces();
    UpdateSoundPieces();
    UpdateResText();
}

void SettingPage::Hide() {
    visible = false;
    SaveToGameData();
    for (auto& p : settingObjects)
        p.first->SetPosition(HIDDEN);
}

void SettingPage::Reset() {
    overlay    = nullptr;
    backboard  = nullptr;
    titleLabel = nullptr;
    musicLabel = soundLabel = resLabel = nullptr;
    musicIcon  = soundIcon  = nullptr;
    musicBarBg = soundBarBg = nullptr;
    musicLeftArrow = musicRightArrow = nullptr;
    soundLeftArrow = soundRightArrow = nullptr;
    for (int i = 0; i < 10; i++) musicPieces[i] = soundPieces[i] = nullptr;
    resIcon = nullptr;
    resText = nullptr;
    resLeftArrow = resRightArrow = nullptr;
    backBtnImg = applyBtnImg = nullptr;
    backLabel  = applyLabel  = nullptr;
    settingObjects.clear();
    visible = false;
}

// ─────────────────────────────────────────────────────────────────────────────
SettingPage::Action SettingPage::HandleClick(float wx, float wy) {
    if (!visible) return Action::NONE;

    // ── Music row ─────────────────────────────────────────────────────────────
    if (areaMusicToggle.Hit(wx, wy)) {
        musicEnabled = !musicEnabled;
        UpdateMusicIcon();
        ApplyMusicVolume();
        return Action::NONE;
    }
    if (areaMusicLeft.Hit(wx, wy) && musicVolume > 0) {
        musicVolume--;
        UpdateMusicPieces();
        ApplyMusicVolume();
        return Action::NONE;
    }
    if (areaMusicRight.Hit(wx, wy) && musicVolume < 10) {
        musicVolume++;
        UpdateMusicPieces();
        ApplyMusicVolume();
        return Action::NONE;
    }

    // ── Sound row ─────────────────────────────────────────────────────────────
    if (areaSoundToggle.Hit(wx, wy)) {
        soundEnabled = !soundEnabled;
        UpdateSoundIcon();
        ApplySoundVolume();
        return Action::NONE;
    }
    if (areaSoundLeft.Hit(wx, wy) && soundVolume > 0) {
        soundVolume--;
        UpdateSoundPieces();
        ApplySoundVolume();
        return Action::NONE;
    }
    if (areaSoundRight.Hit(wx, wy) && soundVolume < 10) {
        soundVolume++;
        UpdateSoundPieces();
        ApplySoundVolume();
        return Action::NONE;
    }

    // ── Resolution row ────────────────────────────────────────────────────────
    if (areaResLeft.Hit(wx, wy)) {
        resolutionIndex = (resolutionIndex + 2) % 3;
        UpdateResText();
        return Action::NONE;
    }
    if (areaResRight.Hit(wx, wy)) {
        resolutionIndex = (resolutionIndex + 1) % 3;
        UpdateResText();
        return Action::NONE;
    }

    // ── Back ──────────────────────────────────────────────────────────────────
    if (areaBack.Hit(wx, wy)) {
        Hide();
        return Action::CLOSE;
    }

    // ── Apply ─────────────────────────────────────────────────────────────────
    if (areaApply.Hit(wx, wy)) {
        ApplyResolution();
        Hide();
        return Action::CLOSE;
    }

    return Action::NONE;
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingPage::HandleHover(float wx, float wy) {
    if (!visible) return;

    if (backBtnImg)
        backBtnImg->SetAlpha(areaBack.Hit(wx, wy) ? 1.0f : 0.65f);

    if (applyBtnImg)
        applyBtnImg->SetAlpha(areaApply.Hit(wx, wy) ? 1.0f : 0.65f);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingPage::UpdateMusicPieces() {
    for (int i = 0; i < 10; i++)
        if (musicPieces[i])
            musicPieces[i]->SetAlpha(i < musicVolume ? 1.0f : 0.0f);
}

void SettingPage::UpdateSoundPieces() {
    for (int i = 0; i < 10; i++)
        if (soundPieces[i])
            soundPieces[i]->SetAlpha(i < soundVolume ? 1.0f : 0.0f);
}

void SettingPage::UpdateMusicIcon() {
    if (musicIcon)
        musicIcon->SetTexture(musicEnabled
            ? DIR + "open_music.png"
            : DIR + "close_music.png");
}

void SettingPage::UpdateSoundIcon() {
    if (soundIcon)
        soundIcon->SetTexture(soundEnabled
            ? DIR + "open_sound.png"
            : DIR + "close_sound.png");
}

void SettingPage::UpdateResText() {
    if (resText) {
        SDL_Color white = { 240, 240, 240, 255 };
        resText->LoadText(RESOLUTIONS[resolutionIndex].label, white, 30);
    }
}

void SettingPage::ApplyMusicVolume() {
    int vol = musicEnabled ? (musicVolume * MIX_MAX_VOLUME / 10) : 0;
    Mix_VolumeMusic(vol);
}

void SettingPage::ApplySoundVolume() {
    int vol = soundEnabled ? (soundVolume * MIX_MAX_VOLUME / 10) : 0;
    Mix_Volume(-1, vol);
}

void SettingPage::ApplyResolution() {
    const ResOption& res = RESOLUTIONS[resolutionIndex];
    SDL_SetWindowSize(window, res.w, res.h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    GameEngine::GetInstance()->GetRenderer()->SetViewPort(0, 0, res.w, res.h);
    GameEngine::GetInstance()->SetWindowSize(res.w, res.h);
}

void SettingPage::SaveToGameData() {
    auto* gd            = GameData::GetInstance();
    gd->musicVolume     = musicVolume;
    gd->musicEnabled    = musicEnabled;
    gd->soundVolume     = soundVolume;
    gd->soundEnabled    = soundEnabled;
    gd->resolutionIndex = resolutionIndex;
}
