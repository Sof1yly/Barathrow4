// EventScene.h
#pragma once
#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "DrawableObject.h"
#include "ImageObject.h"
#include "TextObject.h"
#include "SpriteObject.h"

class EventScene
{
public:
    enum class EffectType
    {
        EXTRA_DRAW,
        GOLD_BONUS,
        START_BARRIER,
        START_OVERCLOCK,
        MAX_HP,
        CURRENCY,
        REMOVE_CARDS
    };

private:
    struct Option
    {
        EffectType  effect;
        std::string title;
        std::string description;
    };

    struct ChoiceBox
    {
        EffectType   effect;
        ImageObject* normalImg   = nullptr;
        ImageObject* hoverImg    = nullptr;
        ImageObject* selectedImg = nullptr;
        TextObject*  label       = nullptr;
        TextObject*  descLabel   = nullptr;
        float minX, maxX, minY, maxY;
    };

    static constexpr int   OFFER_COUNT  = 3;
    static constexpr float SELECT_DELAY = 2000.0f; // ms

    std::vector<Option>          offered;
    std::vector<DrawableObject*> uiObjects;
    std::vector<ChoiceBox>       choiceBoxes;

    bool        active        = false;
    int         hoveredIdx    = -1;
    int         selectedIdx   = -1;
    float       selectTimer   = 0.0f;
    bool        pendingClose  = false;
    EffectType  pendingEffect = EffectType::EXTRA_DRAW;
    SpriteObject* bossSprite  = nullptr;

    std::mt19937 rng;

    bool IsInside(const ChoiceBox& b, float x, float y) const
    {
        return x >= b.minX && x <= b.maxX && y >= b.minY && y <= b.maxY;
    }

    void ShowBoxState(ChoiceBox& b, int state) // 0=normal, 1=hover, 2=selected
    {
        if (b.normalImg)   b.normalImg->SetAlpha(  state == 0 ? 1.0f : 0.0f);
        if (b.hoverImg)    b.hoverImg->SetAlpha(   state == 1 ? 1.0f : 0.0f);
        if (b.selectedImg) b.selectedImg->SetAlpha(state == 2 ? 1.0f : 0.0f);
    }

    void Push(DrawableObject* obj, std::vector<DrawableObject*>& objectsList)
    {
        uiObjects.push_back(obj);
        objectsList.push_back(obj);
    }

    void BuildUI(std::vector<DrawableObject*>& objectsList)
    {
        // Boss pushed first so everything else draws on top of it
        bossSprite = new SpriteObject("../Resource/Texture/Boss/Boss1.png", 8, 11);
        bossSprite->SetAnimationLoop(0, 0, 5, 250);
        bossSprite->SetSize(800.0f, -571.0f);
        bossSprite->SetPosition(glm::vec3(0.0f, 320.0f, 699.0f));
        Push(bossSprite, objectsList);

        // Speech bubble / text box (upper center)
        {
            ImageObject* bg = new ImageObject();
            bg->SetTexture("../Resource/Texture/UI/Event/Text Box.png");
            bg->SetSize(735.0f, -393.0f);
            bg->SetPosition(glm::vec3(0.0f, 100.0f, 700.0f));
            Push(bg, objectsList);
        }

        // Event headline inside the bubble
        {
            TextObject* title = new TextObject();
            SDL_Color white = { 245, 245, 245, 255 };
            title->LoadText("Choose a Blessing", white, 38);
            title->SetPosition(glm::vec3(0.0f, 130.0f, 705.0f));
            Push(title, objectsList);
        }

        const float BOX_W  = 735.0f;
        const float BOX_H  = 110.0f;
        const float step   = BOX_H + 24.0f;
        const float startY = -200.0f;

        for (int i = 0; i < (int)offered.size(); i++)
        {
            float cy = startY - step * i;

            ChoiceBox box;
            box.effect = offered[i].effect;
            box.minX   = -BOX_W * 0.5f;
            box.maxX   =  BOX_W * 0.5f;
            box.minY   = cy - BOX_H * 0.5f;
            box.maxY   = cy + BOX_H * 0.5f;

            auto mkImg = [&](const char* tex, float w, float h, float z, float alpha) -> ImageObject*
            {
                ImageObject* img = new ImageObject();
                img->SetTexture(tex);
                img->SetSize(w, -h);
                img->SetPosition(glm::vec3(0.0f, cy, z));
                img->SetAlpha(alpha);
                Push(img, objectsList);
                return img;
            };

            box.normalImg   = mkImg("../Resource/Texture/UI/Event/Normal choice Box.png",   BOX_W, BOX_H, 710.0f, 1.0f);
            box.hoverImg    = mkImg("../Resource/Texture/UI/Event/Hover choice Box.png",    BOX_W, BOX_H, 710.0f, 0.0f);
            box.selectedImg = mkImg("../Resource/Texture/UI/Event/Selected choice Box.png", BOX_W, BOX_H, 710.0f, 0.0f);

            {
                TextObject* lbl = new TextObject();
                SDL_Color c = { 255, 255, 200, 255 };
                lbl->LoadText(offered[i].title, c, 26);
                lbl->SetPosition(glm::vec3(0.0f, cy + 22.0f, 715.0f));
                box.label = lbl;
                Push(lbl, objectsList);
            }

            {
                TextObject* desc = new TextObject();
                SDL_Color c = { 200, 200, 200, 255 };
                desc->LoadText(offered[i].description, c, 20);
                desc->SetPosition(glm::vec3(0.0f, cy - 22.0f, 715.0f));
                box.descLabel = desc;
                Push(desc, objectsList);
            }

            choiceBoxes.push_back(box);
        }
    }

    void ClearUI(std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : uiObjects)
        {
            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end()) objectsList.erase(it);
            delete obj;
        }
        uiObjects.clear();
        choiceBoxes.clear();
    }

public:
    EventScene() : rng(std::random_device{}()) {}

    void Open(std::vector<DrawableObject*>& objectsList)
    {
        if (active) return;

        static const Option ALL_OPTIONS[] = {
            { EffectType::EXTRA_DRAW,      "Extra Draw",  "Draw an extra card at the start of each turn"   },
            { EffectType::GOLD_BONUS,      "Gold Rush",   "Gain bonus gold at the end of each round"       },
            { EffectType::START_BARRIER,   "Fortified",   "Begin each level with a protective barrier"     },
            { EffectType::START_OVERCLOCK, "Overclock",   "Start each level in an overclocked state"       },
            { EffectType::MAX_HP,          "Vitality",    "Increase your maximum health permanently"       },
            { EffectType::CURRENCY,        "Windfall",    "Receive a windfall of bonus currency"           },
            { EffectType::REMOVE_CARDS,    "Purge",       "Remove unwanted cards from your deck"           },
        };
        static constexpr int N = (int)(sizeof(ALL_OPTIONS) / sizeof(ALL_OPTIONS[0]));

        std::vector<int> idx(N);
        std::iota(idx.begin(), idx.end(), 0);
        std::shuffle(idx.begin(), idx.end(), rng);

        offered.clear();
        for (int i = 0; i < OFFER_COUNT; i++)
            offered.push_back(ALL_OPTIONS[idx[i]]);

        BuildUI(objectsList);
        active       = true;
        hoveredIdx   = -1;
        selectedIdx  = -1;
        selectTimer  = 0.0f;
        pendingClose = false;
    }

    void HandleMouseHover(float x, float y)
    {
        if (!active || selectedIdx >= 0) return;

        int newIdx = -1;
        for (int i = 0; i < (int)choiceBoxes.size(); i++)
            if (IsInside(choiceBoxes[i], x, y)) { newIdx = i; break; }

        if (newIdx == hoveredIdx) return;
        hoveredIdx = newIdx;

        for (int i = 0; i < (int)choiceBoxes.size(); i++)
            ShowBoxState(choiceBoxes[i], i == hoveredIdx ? 1 : 0);
    }

    void HandleMouseClick(float x, float y)
    {
        if (!active || selectedIdx >= 0) return;

        for (int i = 0; i < (int)choiceBoxes.size(); i++)
        {
            if (IsInside(choiceBoxes[i], x, y))
            {
                selectedIdx   = i;
                selectTimer   = 0.0f;
                pendingEffect = choiceBoxes[i].effect;
                ShowBoxState(choiceBoxes[i], 2);
                return;
            }
        }
    }

    void Update(float dt)
    {
        if (!active) return;
        if (bossSprite) bossSprite->Update(dt);
        if (selectedIdx < 0 || pendingClose) return;
        selectTimer += dt;
        if (selectTimer >= SELECT_DELAY)
            pendingClose = true;
    }

    bool       IsReadyToClose()    const { return pendingClose; }
    EffectType GetPendingEffect()  const { return pendingEffect; }

    void Close(std::vector<DrawableObject*>& objectsList)
    {
        if (!active && uiObjects.empty()) return;
        ClearUI(objectsList);
        bossSprite   = nullptr;  // deleted by ClearUI via uiObjects
        offered.clear();
        hoveredIdx   = -1;
        selectedIdx  = -1;
        selectTimer  = 0.0f;
        pendingClose = false;
        active       = false;
    }

    bool IsActive() const { return active; }
};
