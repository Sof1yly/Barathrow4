#include "Button.h"

#include <cmath>

bool Button::isMenu = false;

void Button::setMenu(bool menu) {
    isMenu = menu;
}

bool Button::getMenu() {
    return isMenu;
}

void Button::Init(const std::string& texturePath,
                  const glm::vec3& pos,
                  const glm::vec2& buttonSize,
                  std::vector<DrawableObject*>& objectsList)
{
    position = pos;
    size = buttonSize;

    if (!image)
    {
        image = new ImageObject();
        objectsList.push_back(image);
    }

    image->SetSize(size.x, size.y);
    image->SetPosition(position);
    image->SetTexture(texturePath);
}

bool Button::IsClicked(float worldX, float worldY) const
{
    if (!image) return false;

    float halfW = std::abs(size.x) * 0.5f;
    float halfH = std::abs(size.y) * 0.5f;

    return (worldX >= position.x - halfW && worldX <= position.x + halfW &&
            worldY >= position.y - halfH && worldY <= position.y + halfH);
}

void Button::Reset()
{
    image = nullptr;
    position = glm::vec3(0.0f);
    size = glm::vec2(0.0f);
}
