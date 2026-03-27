#include "Button.h"

#include <cmath>

bool Button::isMenu = false;

void Button::setMenu(bool menu) {
    isMenu = menu;
}

bool Button::getMenu() {
    return isMenu;
}

glm::vec3 Button::HiddenHintPosition()
{
    return glm::vec3(0.0f, 10000.0f, 20.0f);
}

void Button::Init(const std::string& texturePath,const glm::vec3& pos,const glm::vec2& buttonSize,std::vector<DrawableObject*>& objectsList)
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

void Button::InitPreset(Preset preset, std::vector<DrawableObject*>& objectsList)
{
    switch (preset)
    {
    case Preset::ViewDeck:
        Init("../Resource/Texture/UI/ViewDeck.PNG",glm::vec3(710.0f, 500.0f, 0.0f), glm::vec2(80.0f, -80.0f),objectsList);
        hintPosition = glm::vec3(705.0f, 440.0f, 20.0f);
        break;

    case Preset::SkipTurn:
        Init("../Resource/Texture/UI/SkipBut.png",glm::vec3(800.0f, -90.0f, 12.0f),glm::vec2(100.0f, -100.0f),objectsList);
        hintPosition = glm::vec3(740.0f, -15.0f, 20.0f);
        break;

    default:
        break;
    }
}

bool Button::IsClicked(float worldX, float worldY) const
{
    if (!image) return false;

    float halfW = std::abs(size.x) * 0.5f;
    float halfH = std::abs(size.y) * 0.5f;

    return (worldX >= position.x - halfW && worldX <= position.x + halfW &&worldY >= position.y - halfH && worldY <= position.y + halfH);
}

void Button::Reset()
{
    image = nullptr;
    position = glm::vec3(0.0f);
    size = glm::vec2(0.0f);
    hintPosition = glm::vec3(0.0f);
}
