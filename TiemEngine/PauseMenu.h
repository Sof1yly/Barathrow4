#pragma once

#include "ImageObject.h"
#include "TextObject.h"
#include <SDL.h>
#include <vector>

class PauseMenu {
public:
    enum class Action { NONE, RESUME, SETTING, SAVE_QUIT_MAIN, SAVE_QUIT_DESKTOP, ABANDON };

    void Init(std::vector<DrawableObject*>& objectsList);
    void Reset();   // null out pointers without deleting (objectsList owns them)

    void Show(std::vector<DrawableObject*>& objectsList);
    void Hide();
    bool IsVisible() const { return visible; }

    Action HandleClick(float wx, float wy);
    void   HandleHover(float wx, float wy);

private:
    bool visible = false;

    struct PauseButton {
        ImageObject* bg      = nullptr;
        TextObject*  label   = nullptr;
        glm::vec3    pos     = {};
        glm::vec2    size    = {};
        bool         enabled = true;
        bool IsClicked(float wx, float wy) const;
    };

    ImageObject* panel      = nullptr;
    TextObject*  titleText  = nullptr;
    PauseButton  btnResume;
    PauseButton  btnSetting;
    PauseButton  btnAbandon;
    PauseButton  btnSaveQuit;
    PauseButton  btnQuitDesktop;

    // Cached real positions so we can restore them on Show.
    std::vector<std::pair<DrawableObject*, glm::vec3>> pauseObjects;

    static const glm::vec3 HIDDEN;

    void AddPauseObject(DrawableObject* obj, glm::vec3 visiblePos,
                        std::vector<DrawableObject*>& objectsList);
    void InitButton(PauseButton& btn, const std::string& labelStr,
                    SDL_Color color,
                    glm::vec3 pos, glm::vec2 size,
                    std::vector<DrawableObject*>& objectsList);
};
