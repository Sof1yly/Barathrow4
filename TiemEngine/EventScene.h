// EventScene.h
#pragma once
#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "DrawableObject.h"
#include "GameObject.h"
#include "TextObject.h"

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
        float r, g, b;
    };

    struct ClickArea
    {
        EffectType effect;
        float minX, maxX, minY, maxY;
    };

    static constexpr int OFFER_COUNT = 3;

    std::vector<Option>          offered;
    std::vector<DrawableObject*> uiObjects;
    std::vector<ClickArea>       clickAreas;
    bool         active = false;
    std::mt19937 rng;

    bool IsInside(const ClickArea& area, float x, float y) const
    {
        return x >= area.minX && x <= area.maxX && y >= area.minY && y <= area.maxY;
    }

    void BuildUI(std::vector<DrawableObject*>& objectsList)
    {
        {
            GameObject* panel = new GameObject();
            panel->SetSize(1920.0f, 1080.0f);
            panel->SetPosition(glm::vec3(0.0f, 0.0f, 700.0f));
            panel->SetColor(0.0f, 0.0f, 0.0f, 0.82f);
            uiObjects.push_back(panel);
            objectsList.push_back(panel);
        }

        {
            TextObject* title = new TextObject();
            SDL_Color white = { 245, 245, 245, 255 };
            title->LoadText("Choose a Blessing", white, 60);
            title->SetPosition(glm::vec3(0.0f, 370.0f, 705.0f));
            uiObjects.push_back(title);
            objectsList.push_back(title);
        }

        {
            TextObject* sub = new TextObject();
            SDL_Color gray = { 200, 200, 200, 255 };
            sub->LoadText("Pick one to carry through your journey", gray, 28);
            sub->SetPosition(glm::vec3(0.0f, 305.0f, 705.0f));
            uiObjects.push_back(sub);
            objectsList.push_back(sub);
        }

        const float tabW  = 850.0f;
        const float tabH  = 80.0f;
        const float gap   = 20.0f;
        const float step  = tabH + gap;
        const float startY = step * (OFFER_COUNT - 1) * 0.5f;

        for (int i = 0; i < static_cast<int>(offered.size()); i++)
        {
            const Option& opt = offered[i];
            float cy = startY - step * i;

            {
                GameObject* bg = new GameObject();
                bg->SetSize(tabW, tabH);
                bg->SetPosition(glm::vec3(0.0f, cy, 710.0f));
                bg->SetColor(opt.r, opt.g, opt.b, 0.90f);
                uiObjects.push_back(bg);
                objectsList.push_back(bg);
            }

            {
                TextObject* ttl = new TextObject();
                SDL_Color titleColor = { 255, 255, 200, 255 };
                ttl->LoadText(opt.title, titleColor, 30);
                ttl->SetPosition(glm::vec3(-tabW * 0.28f, cy, 715.0f));
                uiObjects.push_back(ttl);
                objectsList.push_back(ttl);
            }

            {
                TextObject* desc = new TextObject();
                SDL_Color descColor = { 230, 230, 230, 255 };
                desc->LoadText(opt.description, descColor, 26);
                desc->SetPosition(glm::vec3(tabW * 0.12f, cy, 715.0f));
                uiObjects.push_back(desc);
                objectsList.push_back(desc);
            }

            ClickArea area;
            area.effect = opt.effect;
            area.minX   = -tabW * 0.5f;
            area.maxX   =  tabW * 0.5f;
            area.minY   = cy - tabH * 0.5f;
            area.maxY   = cy + tabH * 0.5f;
            clickAreas.push_back(area);
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
        clickAreas.clear();
    }

public:
    EventScene() : rng(std::random_device{}()) {}

    void Open(std::vector<DrawableObject*>& objectsList)
    {
        if (active) return;

        static const Option ALL_OPTIONS[] = {
            { EffectType::EXTRA_DRAW,      "Extra Draw",  "+1 card per draw",       0.10f, 0.30f, 0.60f },
            { EffectType::GOLD_BONUS,      "Gold Rush",   "+25% gold gain",         0.50f, 0.40f, 0.05f },
            { EffectType::START_BARRIER,   "Fortified",   "Start with 1 Barrier",   0.20f, 0.50f, 0.20f },
            { EffectType::START_OVERCLOCK, "Overclock",   "Start with 3 Overclock", 0.60f, 0.15f, 0.15f },
            { EffectType::MAX_HP,          "Vitality",    "Gain 10 max HP",         0.15f, 0.50f, 0.40f },
            { EffectType::CURRENCY,        "Windfall",    "Gain 300 currency",      0.50f, 0.45f, 0.10f },
            { EffectType::REMOVE_CARDS,    "Purge",       "Remove 2 deck cards",    0.40f, 0.10f, 0.40f },
        };
        static constexpr int OPTION_COUNT = static_cast<int>(sizeof(ALL_OPTIONS) / sizeof(ALL_OPTIONS[0]));

        std::vector<int> indices(OPTION_COUNT);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);

        offered.clear();
        for (int i = 0; i < OFFER_COUNT; i++)
            offered.push_back(ALL_OPTIONS[indices[i]]);

        BuildUI(objectsList);
        active = true;
    }

    // Returns true when an option was clicked; sets outEffect to the chosen effect.
    bool HandleMouseClick(float x, float y, EffectType& outEffect)
    {
        if (!active) return false;
        for (const ClickArea& area : clickAreas)
        {
            if (IsInside(area, x, y))
            {
                outEffect = area.effect;
                return true;
            }
        }
        return false;
    }

    void Close(std::vector<DrawableObject*>& objectsList)
    {
        if (!active && uiObjects.empty()) return;
        ClearUI(objectsList);
        offered.clear();
        active = false;
    }

    bool IsActive() const { return active; }
};
