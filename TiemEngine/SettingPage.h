#pragma once

#include "ImageObject.h"
#include "TextObject.h"
#include <vector>

class SettingPage {
public:
    enum class Action { NONE, CLOSE };

    void Init(std::vector<DrawableObject*>& objectsList);
    void Reset();
    void Show(std::vector<DrawableObject*>& objectsList);
    void Hide();
    bool IsVisible() const { return visible; }

    Action HandleClick(float wx, float wy);
    void   HandleHover(float wx, float wy);

private:
    bool visible = false;
    static const glm::vec3 HIDDEN;

    struct Area {
        float cx, cy, hw, hh;
        bool Hit(float wx, float wy) const {
            return wx >= cx - hw && wx <= cx + hw &&
                   wy >= cy - hh && wy <= cy + hh;
        }
    };

    std::vector<std::pair<DrawableObject*, glm::vec3>> settingObjects;
    void AddObj(DrawableObject* obj, glm::vec3 visPos, std::vector<DrawableObject*>& lst);

    ImageObject* overlay   = nullptr;
    ImageObject* backboard = nullptr;
    TextObject*  titleLabel = nullptr;

    // Row labels (left side)
    TextObject* musicLabel = nullptr;
    TextObject* soundLabel = nullptr;
    TextObject* resLabel   = nullptr;

    // Music row
    ImageObject* musicIcon       = nullptr;
    ImageObject* musicBarBg      = nullptr;
    ImageObject* musicLeftArrow  = nullptr;
    ImageObject* musicRightArrow = nullptr;
    ImageObject* musicPieces[10] = {};
    Area areaMusicToggle = {}, areaMusicLeft = {}, areaMusicRight = {};

    // Sound row
    ImageObject* soundIcon       = nullptr;
    ImageObject* soundBarBg      = nullptr;
    ImageObject* soundLeftArrow  = nullptr;
    ImageObject* soundRightArrow = nullptr;
    ImageObject* soundPieces[10] = {};
    Area areaSoundToggle = {}, areaSoundLeft = {}, areaSoundRight = {};

    // Resolution row
    ImageObject* resIcon       = nullptr;
    TextObject*  resText       = nullptr;
    ImageObject* resLeftArrow  = nullptr;
    ImageObject* resRightArrow = nullptr;
    Area areaResLeft = {}, areaResRight = {};

    // Buttons
    ImageObject* backBtnImg  = nullptr;
    ImageObject* applyBtnImg = nullptr;
    TextObject*  backLabel   = nullptr;
    TextObject*  applyLabel  = nullptr;
    Area areaBack = {}, areaApply = {};

    // State
    int  musicVolume     = 8;
    bool musicEnabled    = true;
    int  soundVolume     = 8;
    bool soundEnabled    = true;
    int  resolutionIndex = 2;   // 0=1920x1080  1=1600x900  2=1280x720

    struct ResOption { int w, h; const char* label; };
    static const ResOption RESOLUTIONS[3];

    void UpdateMusicPieces();
    void UpdateSoundPieces();
    void UpdateMusicIcon();
    void UpdateSoundIcon();
    void UpdateResText();
    void ApplyMusicVolume();
    void ApplySoundVolume();
    void ApplyResolution();
    void SaveToGameData();
};
