#pragma once

#include "GameEngine.h"
#include "GameData.h"
#include "ImageObject.h"
#include "TextObject.h"
#include "Level.h"
#include <vector>
#include <string>

struct MenuButton {
    ImageObject* bg = nullptr;
    TextObject* label = nullptr;
    glm::vec3 pos;
    glm::vec2 size;

    bool IsClicked(float wx, float wy) const {
        float hw = size.x * 0.5f;
        float hh = size.y * 0.5f;
        return wx >= pos.x - hw && wx <= pos.x + hw &&
               wy >= pos.y - hh && wy <= pos.y + hh;
    }
};

class MainMenu : public Level {
private:
    std::vector<DrawableObject*> objectsList;

    MenuButton btnStart;
    MenuButton btnContinue;
    MenuButton btnAbandon;
    MenuButton btnSettings;
    MenuButton btnQuit;

    void InitButton(MenuButton& btn, const std::string& text,
                    const std::string& texPath, SDL_Color labelColor,
                    glm::vec3 pos, glm::vec2 size);
    void GetRealMousePos(int x, int y, float& rx, float& ry) const;

public:
    virtual void LevelLoad() override;
    virtual void LevelInit() override;
    virtual void LevelUpdate() override;
    virtual void LevelDraw() override;
    virtual void LevelFree() override;
    virtual void LevelUnload() override;
    virtual void HandleKey(char key) override;
    virtual void HandleMouse(int type, int x, int y) override;
};
