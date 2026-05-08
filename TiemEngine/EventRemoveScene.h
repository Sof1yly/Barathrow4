// EventRemoveScene.h
#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "Button.h"
#include "Card.h"
#include "CardSystem.h"
#include "DrawableObject.h"
#include "GameObject.h"
#include "ImageObject.h"
#include "TextObject.h"

// Shows the player's full deck and lets them click cards to remove them.
// Used by the Purge event blessing (removes 2 cards of player's choice).
class EventRemoveScene
{
private:
    struct CardSlot
    {
        Card* card = nullptr;
        float minX = 0, maxX = 0, minY = 0, maxY = 0;
    };

    std::vector<DrawableObject*> staticObjects;
    std::vector<DrawableObject*> cardObjects;
    std::vector<CardSlot>        candidates;
    std::vector<Card*>           showCards;

    TextObject* titleLabel = nullptr;
    TextObject* pageLabel  = nullptr;
    Button leftBtn, rightBtn;

    int  currentPage = 0;
    int  totalPages  = 0;
    int  removesLeft = 0;
    bool active      = false;

    static constexpr int   CARDS_PER_PAGE = 5;
    static constexpr float CARD_W   = 280.0f;
    static constexpr float CARD_H   = 410.0f;
    static constexpr float SPACING  = 320.0f;
    static constexpr float CENTER_Y = 20.0f;

    static ImageObject* CloneImg(ImageObject* src, const glm::vec3& pos, float w, float h)
    {
        if (!src) return nullptr;
        ImageObject* copy = new ImageObject();
        copy->SetTextureId(src->GetTextureId());
        copy->SetPosition(pos);
        copy->SetSize(w, h);
        return copy;
    }

    void Erase(DrawableObject* obj, std::vector<DrawableObject*>& objectsList)
    {
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
    }

    void ClearCardObjects(std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : cardObjects)
        {
            Erase(obj, objectsList);
            delete obj;
        }
        cardObjects.clear();
        candidates.clear();
    }

    void UpdateTitle()
    {
        if (!titleLabel) return;
        SDL_Color c = { 255, 215, 90, 255 };
        std::string s = "Remove " + std::to_string(removesLeft)
                      + " more card" + (removesLeft != 1 ? "s" : "");
        titleLabel->LoadText(s, c, 40);
    }

    void RebuildPage(std::vector<DrawableObject*>& objectsList)
    {
        ClearCardObjects(objectsList);
        UpdateTitle();

        if (pageLabel)
        {
            SDL_Color c = { 200, 200, 200, 255 };
            pageLabel->LoadText(
                "Page " + std::to_string(currentPage + 1) + " / "
                        + std::to_string(totalPages), c, 22);
        }

        auto push = [&](DrawableObject* obj)
        {
            if (obj) { cardObjects.push_back(obj); objectsList.push_back(obj); }
        };

        int startIdx = currentPage * CARDS_PER_PAGE;
        int endIdx   = std::min(startIdx + CARDS_PER_PAGE, (int)showCards.size());
        int count    = endIdx - startIdx;
        float startX = -0.5f * static_cast<float>(count - 1) * SPACING;

        for (int i = 0; i < count; ++i)
        {
            Card* card = showCards[startIdx + i];
            if (!card) continue;
            if (!card->HasVisuals()) card->CreateVisuals();

            float       cardX   = startX + i * SPACING;
            glm::vec3   cardPos(cardX, CENTER_Y, 818.0f);

            push(CloneImg(card->GetBackground(),  cardPos, CARD_W, -CARD_H));
            push(CloneImg(card->GetStarOverlay(), cardPos, CARD_W, -CARD_H));
            push(CloneImg(card->GetTypeIcon(),    cardPos, CARD_W, -CARD_H));
            push(CloneImg(card->GetVisual(),      cardPos, CARD_W, -CARD_H));
            push(CloneImg(card->GetCardFrame(),   cardPos, CARD_W, -CARD_H));

            if (card->GetNameText())
            {
                TextObject* src  = card->GetNameText();
                TextObject* copy = new TextObject();
                copy->SetTextureId(src->GetTextureId());
                float nameW = src->GetSize().x - 2.5f;
                float nameH = src->GetSize().y - 1.5f;
                nameH = (nameH < 0.0f) ? nameH + 2.0f : nameH - 2.0f;
                copy->SetSize(nameW, nameH);
                glm::vec3 local = src->GetLocalPosition();
                float leftX = (cardX - CARD_W * 0.5f) + (local.x * CARD_W);
                float centX = leftX + copy->GetSize().x * 0.5f + 12.0f;
                copy->SetPosition(glm::vec3(centX, CENTER_Y + local.y * CARD_H, 822.0f));
                push(copy);
            }

            if (card->GetDescriptionText())
            {
                TextObject* src  = card->GetDescriptionText();
                TextObject* copy = new TextObject();
                copy->SetTextureId(src->GetTextureId());
                copy->SetSize(src->GetSize().x, src->GetSize().y);
                glm::vec3 local = src->GetLocalPosition();
                float leftX = (cardX - CARD_W * 0.5f) + (local.x * CARD_W);
                float centX = leftX + copy->GetSize().x * 0.5f;
                copy->SetPosition(glm::vec3(centX, CENTER_Y + local.y * CARD_H, 822.0f));
                push(copy);
            }

            CardSlot slot;
            slot.card = card;
            slot.minX = cardX - CARD_W * 0.5f;
            slot.maxX = cardX + CARD_W * 0.5f;
            slot.minY = CENTER_Y - CARD_H * 0.5f;
            slot.maxY = CENTER_Y + CARD_H * 0.5f;
            candidates.push_back(slot);
        }
    }

public:
    void Open(CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList, int count)
    {
        if (active) return;

        const std::vector<Card*>& allCards = cardSystem.GetFullCollection();
        showCards.clear();
        for (Card* c : allCards)
            if (c && !c->isEnergyCard()) showCards.push_back(c);

        if (showCards.empty()) return;

        removesLeft = std::min(count, static_cast<int>(showCards.size()));
        currentPage = 0;
        totalPages  = (static_cast<int>(showCards.size()) + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;

        auto pushStatic = [&](DrawableObject* obj)
        {
            if (obj) { staticObjects.push_back(obj); objectsList.push_back(obj); }
        };

        // Dark overlay
        GameObject* bg = new GameObject();
        bg->SetSize(1920.0f, 1080.0f);
        bg->SetPosition(glm::vec3(0.0f, 0.0f, 815.0f));
        bg->SetColor(0.0f, 0.0f, 0.0f, 0.96f);
        pushStatic(bg);

        // Title (text updated each removal)
        titleLabel = new TextObject();
        titleLabel->SetPosition(glm::vec3(0.0f, 420.0f, 820.0f));
        pushStatic(titleLabel);

        // Subtitle
        TextObject* sub = new TextObject();
        SDL_Color gray = { 200, 200, 200, 255 };
        sub->LoadText("Click a card to remove it from your deck", gray, 26);
        sub->SetPosition(glm::vec3(0.0f, 365.0f, 820.0f));
        pushStatic(sub);

        // Page label
        pageLabel = new TextObject();
        pageLabel->SetPosition(glm::vec3(0.0f, 315.0f, 820.0f));
        pushStatic(pageLabel);

        leftBtn.Init("../Resource/Texture/UI/Left.png",
                     glm::vec3(-870.0f, 20.0f, 820.0f),
                     glm::vec2(80.0f, -80.0f), objectsList);
        rightBtn.Init("../Resource/Texture/UI/Right.png",
                      glm::vec3(870.0f, 20.0f, 820.0f),
                      glm::vec2(80.0f, -80.0f), objectsList);

        active = true;
        RebuildPage(objectsList);
    }

    // Returns true if the click was handled.
    // Check IsActive() afterwards — false means all removals are done.
    bool HandleMouseClick(float x, float y, CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
    {
        if (!active) return false;

        if (leftBtn.IsClicked(x, y))
        {
            if (currentPage > 0) { currentPage--; RebuildPage(objectsList); }
            return true;
        }
        if (rightBtn.IsClicked(x, y))
        {
            if (currentPage < totalPages - 1) { currentPage++; RebuildPage(objectsList); }
            return true;
        }

        for (auto& slot : candidates)
        {
            if (x < slot.minX || x > slot.maxX || y < slot.minY || y > slot.maxY) continue;
            if (!slot.card) continue;

            cardSystem.RemoveOneCard(slot.card->getName());
            auto it = std::find(showCards.begin(), showCards.end(), slot.card);
            if (it != showCards.end()) showCards.erase(it);
            removesLeft--;

            std::cout << "[Event Purge] Removed '" << slot.card->getName()
                      << "', " << removesLeft << " removal(s) left.\n";

            if (removesLeft <= 0 || showCards.empty())
            {
                Close(objectsList);
            }
            else
            {
                totalPages = (static_cast<int>(showCards.size()) + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
                if (currentPage >= totalPages) currentPage = totalPages - 1;
                RebuildPage(objectsList);
            }
            return true;
        }
        return false;
    }

    void Close(std::vector<DrawableObject*>& objectsList)
    {
        ClearCardObjects(objectsList);

        for (DrawableObject* obj : staticObjects)
        {
            Erase(obj, objectsList);
            delete obj;
        }
        staticObjects.clear();

        auto clearBtn = [&](Button& btn)
        {
            ImageObject* img = btn.GetImage();
            if (!img) { btn.Reset(); return; }
            Erase(img, objectsList);
            delete img;
            btn.Reset();
        };
        clearBtn(leftBtn);
        clearBtn(rightBtn);

        showCards.clear();
        titleLabel = nullptr;
        pageLabel  = nullptr;
        active     = false;
    }

    bool IsActive() const { return active; }
};
