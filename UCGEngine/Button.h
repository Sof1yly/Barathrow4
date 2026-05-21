#pragma once

#include <string>
#include <vector>

#include "ImageObject.h"
#include "DrawableObject.h"

class Button {
public:
    enum class Preset {
        None,
        ViewDeck,
        SkipTurn
    };

    static void setMenu(bool menu);
    static bool getMenu();
    static glm::vec3 HiddenHintPosition();

    Button() = default;

    void Init(const std::string& texturePath,
              const glm::vec3& position,
              const glm::vec2& size,
              std::vector<DrawableObject*>& objectsList);
    void InitPreset(Preset preset, std::vector<DrawableObject*>& objectsList);

    bool IsClicked(float worldX, float worldY) const;
    void Reset();

    ImageObject* GetImage() const { return image; }
    const glm::vec3& GetHintPosition() const { return hintPosition; }

private:
    static bool isMenu;

    ImageObject* image = nullptr;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec2 size = glm::vec2(0.0f);
    glm::vec3 hintPosition = glm::vec3(0.0f);
};
