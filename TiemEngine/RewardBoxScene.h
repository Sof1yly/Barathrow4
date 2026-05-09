// RewardBoxScene.h
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "DrawableObject.h"
#include "GameObject.h"
#include "TextObject.h"

class RewardBoxScene
{
private:
    struct Box
    {
        float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
        bool claimed = false;
        std::vector<DrawableObject*> objects;
    };

    std::vector<DrawableObject*> uiObjects;
    Box coinBox;
    Box cardBox;
    bool active = false;

    bool IsInBox(const Box& box, float x, float y) const
    {
        return x >= box.minX && x <= box.maxX && y >= box.minY && y <= box.maxY;
    }

    void MakeBox(Box& box, float cx, float cy, float w, float h,
                 float r, float g, float b,
                 const std::string& titleStr, SDL_Color titleColor,
                 const std::string& bodyStr, SDL_Color bodyColor,
                 std::vector<DrawableObject*>& objectsList,
                 float z = 710.0f)
    {
        auto AddObj = [&](DrawableObject* obj) {
            uiObjects.push_back(obj);
            objectsList.push_back(obj);
            box.objects.push_back(obj);
        };

        GameObject* bg = new GameObject();
        bg->SetSize(w, h);
        bg->SetPosition(glm::vec3(cx, cy, z));
        bg->SetColor(r, g, b, 0.9f);
        AddObj(bg);

        TextObject* label = new TextObject();
        label->LoadText(titleStr, titleColor, 30);
        label->SetPosition(glm::vec3(cx - w * 0.28f, cy, z + 5.0f));
        AddObj(label);

        TextObject* body = new TextObject();
        body->LoadText(bodyStr, bodyColor, 32);
        body->SetPosition(glm::vec3(cx + w * 0.12f, cy, z + 5.0f));
        AddObj(body);

        box.minX = cx - w * 0.5f;
        box.maxX = cx + w * 0.5f;
        box.minY = cy - h * 0.5f;
        box.maxY = cy + h * 0.5f;
    }

    void RemoveBox(Box& box, std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : box.objects)
        {
            auto it = std::find(uiObjects.begin(), uiObjects.end(), obj);
            if (it != uiObjects.end()) uiObjects.erase(it);
            auto it2 = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it2 != objectsList.end()) objectsList.erase(it2);
            delete obj;
        }
        box.objects.clear();
        box.minX = box.maxX = box.minY = box.maxY = 0.0f;
    }

public:
    void Open(int coins, std::vector<DrawableObject*>& objectsList)
    {
        if (active) return;
        active = true;
        coinBox.claimed = false;
        cardBox.claimed = false;

        // Dark overlay
        GameObject* panel = new GameObject();
        panel->SetSize(1920.0f, 1080.0f);
        panel->SetPosition(glm::vec3(0.0f, 0.0f, 700.0f));
        panel->SetColor(0.0f, 0.0f, 0.0f, 0.75f);
        uiObjects.push_back(panel);
        objectsList.push_back(panel);

        // Title
        TextObject* title = new TextObject();
        SDL_Color white = { 245, 245, 245, 255 };
        title->LoadText("Rewards!", white, 60);
        title->SetPosition(glm::vec3(0.0f, 350.0f, 705.0f));
        uiObjects.push_back(title);
        objectsList.push_back(title);

        const float tabW = 850.0f;
        const float tabH = 85.0f;
        const float gap  = 25.0f;
        const float step = tabH + gap;

        // Card reward tab (top)
        {
            SDL_Color blue = { 150, 200, 255, 255 };
            SDL_Color cardBody = { 200, 230, 255, 255 };
            MakeBox(cardBox, 0.0f, step * 0.5f, tabW, tabH, 0.1f, 0.25f, 0.65f,
                    "Card Reward", blue, "Choose a card", cardBody, objectsList);
        }

        // Coin tab (bottom)
        {
            SDL_Color gold = { 255, 220, 50, 255 };
            SDL_Color coinBody = { 255, 255, 200, 255 };
            MakeBox(coinBox, 0.0f, -step * 0.5f, tabW, tabH, 0.55f, 0.45f, 0.05f,
                    "Coins", gold, std::to_string(coins) + " coins", coinBody, objectsList);
        }
    }

    void Close(std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : uiObjects)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end()) objectsList.erase(it);
            delete obj;
        }
        uiObjects.clear();
        coinBox = Box();
        cardBox = Box();
        active = false;
    }

    // Returns 0=nothing, 1=coin tab clicked, 2=card tab clicked
    int HandleClick(float x, float y, std::vector<DrawableObject*>& objectsList)
    {
        if (!active) return 0;
        if (!coinBox.claimed && IsInBox(coinBox, x, y))
        {
            coinBox.claimed = true;
            RemoveBox(coinBox, objectsList);
            return 1;
        }
        if (!cardBox.claimed && IsInBox(cardBox, x, y))
        {
            RemoveBox(cardBox, objectsList);
            return 2;
        }
        return 0;
    }

    void ClaimCard()
    {
        cardBox.claimed = true;
    }

    bool IsDone() const
    {
        return active && coinBox.claimed && cardBox.claimed;
    }

    bool IsActive() const { return active; }
};
