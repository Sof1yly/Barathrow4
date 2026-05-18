// RewardBoxScene.h
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Button.h"
#include "DrawableObject.h"
#include "GameObject.h"
#include "ImageObject.h"
#include "TextObject.h"

class RewardBoxScene
{
private:
    struct RewardRow
    {
        float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
        bool claimed = false;
        bool hovered = false;
        ImageObject* btnImage = nullptr;
        std::vector<DrawableObject*> objects;
    };

    std::vector<DrawableObject*> uiObjects;
    RewardRow coinRow;
    RewardRow cardRow;
    Button skipButton;
    bool active = false;

    static constexpr float kBoardW  = 675.0f;
    static constexpr float kBoardH  = 858.0f;
    static constexpr float kBannerW = 551.0f;
    static constexpr float kBannerH = 123.0f;
    static constexpr float kBtnW    = 497.0f;
    static constexpr float kBtnH    = 122.0f;
    static constexpr float kIconSz  = 192.0f;
    static constexpr float kSkipW   = 231.0f;
    static constexpr float kSkipH   = 188.0f;

    bool IsInRow(const RewardRow& row, float x, float y) const
    {
        return x >= row.minX && x <= row.maxX && y >= row.minY && y <= row.maxY;
    }

    void AddImage(const std::string& path, float cx, float cy, float z,
                  float w, float h,
                  std::vector<DrawableObject*>& objectsList)
    {
        ImageObject* img = new ImageObject();
        img->SetTexture(path);
        img->SetSize(w, h);
        img->SetPosition(glm::vec3(cx, cy, z));
        uiObjects.push_back(img);
        objectsList.push_back(img);
    }

    void BuildRow(RewardRow& row,
                  float cy,
                  const std::string& iconPath,
                  const std::string& labelStr,
                  SDL_Color labelColor,
                  std::vector<DrawableObject*>& objectsList,
                  float z = 710.0f)
    {
        auto Track = [&](DrawableObject* obj)
        {
            uiObjects.push_back(obj);
            objectsList.push_back(obj);
            row.objects.push_back(obj);
        };

        // button_icon row background
        ImageObject* btn = new ImageObject();
        btn->SetTexture("../Resource/Texture/UI/RewardScene/button_icon.png");
        btn->SetSize(kBtnW, -kBtnH);
        btn->SetPosition(glm::vec3(0.0f, cy, z));
        Track(btn);
        row.btnImage = btn;

        // reward icon pinned to the left of the button (rendered at half native size)
        const float iconRender = kIconSz * 0.5f;
        const float iconX = -(kBtnW * 0.5f) + iconRender * 0.5f + 15.0f;
        ImageObject* icon = new ImageObject();
        icon->SetTexture(iconPath);
        icon->SetSize(iconRender, -iconRender);
        icon->SetPosition(glm::vec3(iconX, cy, z + 2.0f));
        Track(icon);

        // label text centred on the button_icon
        TextObject* label = new TextObject();
        label->LoadText(labelStr, labelColor, 28);
        label->SetPosition(glm::vec3(0.0f, cy - 6.0f, z + 3.0f));
        Track(label);

        row.minX = -(kBtnW * 0.5f);
        row.maxX =  (kBtnW * 0.5f);
        row.minY = cy - kBtnH * 0.5f;
        row.maxY = cy + kBtnH * 0.5f;
    }

    void RemoveRow(RewardRow& row, std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : row.objects)
        {
            auto it = std::find(uiObjects.begin(), uiObjects.end(), obj);
            if (it != uiObjects.end()) uiObjects.erase(it);
            auto it2 = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it2 != objectsList.end()) objectsList.erase(it2);
            delete obj;
        }
        row.objects.clear();
        row.minX = row.maxX = row.minY = row.maxY = 0.0f;
    }

    void RemoveSkipButton(std::vector<DrawableObject*>& objectsList)
    {
        ImageObject* img = skipButton.GetImage();
        if (!img) { skipButton.Reset(); return; }
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) objectsList.erase(it);
        delete img;
        skipButton.Reset();
    }

    static void ApplyRowHover(RewardRow& row, bool nowHovered)
    {
        if (nowHovered == row.hovered) return;
        row.hovered = nowHovered;
        if (!row.btnImage) return;
        if (nowHovered)
            row.btnImage->SetColor(1.4f, 1.4f, 1.0f);
        else
            row.btnImage->SetColor(1.0f, 1.0f, 1.0f);
    }

public:
    void HandleHover(float x, float y)
    {
        if (!active) return;
        ApplyRowHover(coinRow, !coinRow.claimed && IsInRow(coinRow, x, y));
        ApplyRowHover(cardRow, !cardRow.claimed && IsInRow(cardRow, x, y));
    }

    void Open(int coins, std::vector<DrawableObject*>& objectsList)
    {
        if (active) return;
        active = true;
        coinRow.claimed = false;
        cardRow.claimed = false;

        // Dim overlay behind the board
        GameObject* overlay = new GameObject();
        overlay->SetSize(1920.0f, 1080.0f);
        overlay->SetPosition(glm::vec3(0.0f, 0.0f, 698.0f));
        overlay->SetColor(0.0f, 0.0f, 0.0f, 0.65f);
        uiObjects.push_back(overlay);
        objectsList.push_back(overlay);

        // Backboard — bottom-most scene layer
        AddImage("../Resource/Texture/UI/RewardScene/backboard.png",
                 0.0f, 0.0f, 700.0f, kBoardW, -kBoardH, objectsList);

        // loot_banner pinned near the top of the backboard
        const float bannerY = kBoardH * 0.5f - kBannerH * 0.5f - 8.0f;
        AddImage("../Resource/Texture/UI/RewardScene/loot_banner.png",
                 0.0f, bannerY, 705.0f, kBannerW, -kBannerH, objectsList);

        // "LOOT" text centred on the banner
        TextObject* lootText = new TextObject();
        SDL_Color white = { 245, 245, 245, 255 };
        lootText->LoadText("LOOT", white, 56);
        lootText->SetPosition(glm::vec3(0.0f, bannerY - 5.0f, 706.0f));
        uiObjects.push_back(lootText);
        objectsList.push_back(lootText);

        // Reward rows stacked below the banner
        const float belowBanner = bannerY - kBannerH * 0.5f;
        const float rowGap = 22.0f;
        const float coinRowY = belowBanner - rowGap - kBtnH * 0.5f;
        const float cardRowY = coinRowY - kBtnH - rowGap;

        const char* currencyPath;
        if      (coins >= 500) currencyPath = "../Resource/Texture/UI/RewardScene/currency_3.png";
        else if (coins >= 150) currencyPath = "../Resource/Texture/UI/RewardScene/currency_2.png";
        else                   currencyPath = "../Resource/Texture/UI/RewardScene/currency_1.png";

        SDL_Color gold     = { 255, 215,   0, 255 };
        SDL_Color cardBlue = { 160, 210, 255, 255 };

        BuildRow(coinRow, coinRowY, currencyPath,
                 std::to_string(coins) + " Gold", gold, objectsList);

        BuildRow(cardRow, cardRowY,
                 "../Resource/Texture/UI/RewardScene/normal_card.png",
                 "Choose Card", cardBlue, objectsList);

        // Skip button — outside the backboard, bottom-right
        const float skipX =  kBoardW * 0.5f + kSkipW * 0.5f + 10.0f;
        const float skipY = -(kBoardH * 0.5f - kSkipH * 0.5f - 10.0f);
        skipButton.Init(
            "../Resource/Texture/UI/RewardScene/Skip.PNG",
            glm::vec3(skipX, skipY, 715.0f),
            glm::vec2(kSkipW, -kSkipH),
            objectsList
        );
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
        RemoveSkipButton(objectsList);
        coinRow = RewardRow();
        cardRow = RewardRow();
        active = false;
    }

    // Returns: 0 = nothing, 1 = coin row clicked, 2 = card row clicked, 3 = skip
    int HandleClick(float x, float y, std::vector<DrawableObject*>& objectsList)
    {
        if (!active) return 0;

        if (skipButton.IsClicked(x, y))
        {
            RemoveSkipButton(objectsList);
            if (!coinRow.claimed) { coinRow.claimed = true; RemoveRow(coinRow, objectsList); }
            cardRow.claimed = true;
            RemoveRow(cardRow, objectsList);
            return 3;
        }

        if (!coinRow.claimed && IsInRow(coinRow, x, y))
        {
            coinRow.claimed = true;
            RemoveRow(coinRow, objectsList);
            return 1;
        }

        if (!cardRow.claimed && IsInRow(cardRow, x, y))
        {
            RemoveRow(cardRow, objectsList);
            return 2;
        }

        return 0;
    }

    void ClaimCard() { cardRow.claimed = true; }

    bool IsDone()  const { return active && coinRow.claimed && cardRow.claimed; }
    bool IsActive() const { return active; }
};
