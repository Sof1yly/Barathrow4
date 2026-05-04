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
        GameObject* bg = nullptr;
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
                 const std::string& hintStr,
                 std::vector<DrawableObject*>& objectsList,
                 float z = 710.0f)
    {
        GameObject* bg = new GameObject();
        bg->SetSize(w, h);
        bg->SetPosition(glm::vec3(cx, cy, z));
        bg->SetColor(r, g, b, 0.9f);
        uiObjects.push_back(bg);
        objectsList.push_back(bg);
        box.bg = bg;

        TextObject* label = new TextObject();
        label->LoadText(titleStr, titleColor, 36);
        label->SetPosition(glm::vec3(cx, cy + h * 0.27f, z + 5.0f));
        uiObjects.push_back(label);
        objectsList.push_back(label);

        TextObject* body = new TextObject();
        body->LoadText(bodyStr, bodyColor, 42);
        body->SetPosition(glm::vec3(cx, cy, z + 5.0f));
        uiObjects.push_back(body);
        objectsList.push_back(body);

        TextObject* hint = new TextObject();
        SDL_Color hintColor = { 200, 200, 200, 255 };
        hint->LoadText(hintStr, hintColor, 24);
        hint->SetPosition(glm::vec3(cx, cy - h * 0.3f, z + 5.0f));
        uiObjects.push_back(hint);
        objectsList.push_back(hint);

        box.minX = cx - w * 0.5f;
        box.maxX = cx + w * 0.5f;
        box.minY = cy - h * 0.5f;
        box.maxY = cy + h * 0.5f;
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

        const float boxW = 340.0f;
        const float boxH = 300.0f;
        const float gap = 80.0f;
        const float coinX = (boxW * 0.5f) + (gap * 0.5f);
        const float cardX = -coinX;

        // Coin box (right side)
        {
            SDL_Color gold = { 255, 220, 50, 255 };
            SDL_Color coinBody = { 255, 255, 200, 255 };
            MakeBox(coinBox, coinX, 0.0f, boxW, boxH, 0.55f, 0.45f, 0.05f,
                    "Coins", gold, std::to_string(coins) + " coins", coinBody,
                    "Click to collect", objectsList);
        }

        // Card reward box (left side)
        {
            SDL_Color blue = { 150, 200, 255, 255 };
            SDL_Color cardBody = { 200, 230, 255, 255 };
            MakeBox(cardBox, cardX, 0.0f, boxW, boxH, 0.1f, 0.25f, 0.65f,
                    "Card Reward", blue, "Choose a card", cardBody,
                    "Click to choose", objectsList);
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

    // Returns 0=nothing, 1=coin box clicked, 2=card box clicked
    int HandleClick(float x, float y)
    {
        if (!active) return 0;
        if (!coinBox.claimed && IsInBox(coinBox, x, y))
        {
            coinBox.claimed = true;
            if (coinBox.bg) coinBox.bg->SetColor(0.28f, 0.22f, 0.02f, 0.5f);
            return 1;
        }
        if (!cardBox.claimed && IsInBox(cardBox, x, y))
            return 2;
        return 0;
    }

    void ClaimCard()
    {
        cardBox.claimed = true;
        if (cardBox.bg) cardBox.bg->SetColor(0.05f, 0.12f, 0.32f, 0.5f);
    }

    bool IsDone() const
    {
        return active && coinBox.claimed && cardBox.claimed;
    }

    bool IsActive() const { return active; }
};
