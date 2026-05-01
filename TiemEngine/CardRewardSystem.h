// CardRewardSystem.h

#pragma once

#include <algorithm>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "Action.h"
#include "AttackAction.h"
#include "BuffAction.h"
#include "CardSystem.h"
#include "DebuffAction.h"
#include "DrawableObject.h"
#include "EnergyAction.h"
#include "GameDataLoader.h"
#include "GameObject.h"
#include "MoveAction.h"
#include "TextObject.h"
#include "Button.h"

class CardRewardSystem
{
private:
    struct OptionArea
    {
        Card* card = nullptr;
        float minX = 0.0f;
        float maxX = 0.0f;
        float minY = 0.0f;
        float maxY = 0.0f;
    };

    GameDataLoader rewardLoader;
    std::vector<Card*> rewardPool;
    std::vector<Card*> offeredCards;
    std::vector<Card*> ownedRewardCards;

    std::vector<DrawableObject*> uiObjects;
    std::vector<OptionArea> optionAreas;
    Button skipButton;

    bool poolLoaded = false;
    bool active = false;

    std::mt19937 rng;

    // Rarity weights used for the 3 reward slots: com=60%, rar=30%, leg=10%
    static constexpr int RARITY_BUCKET_COM = 0;
    static constexpr int RARITY_BUCKET_RAR = 1;
    static constexpr int RARITY_BUCKET_LEG = 2;

    void BuildOffer()
    {
        offeredCards.clear();

        if (!poolLoaded)
        {
            return;
        }

        // Sort pool into the 3 offered rarity buckets (sta / misc are not rewarded)
        std::vector<Card*> buckets[3];
        for (Card* c : rewardPool)
        {
            if (!c || c->isEnergyCard()) continue;
            const std::string& r = c->getRarityCode();
            if      (r == "com") buckets[RARITY_BUCKET_COM].push_back(c);
            else if (r == "rar") buckets[RARITY_BUCKET_RAR].push_back(c);
            else if (r == "leg") buckets[RARITY_BUCKET_LEG].push_back(c);
        }

        // Shuffle each bucket independently
        for (auto& b : buckets)
            std::shuffle(b.begin(), b.end(), rng);

        // Weighted roll: com = 60, rar = 30, leg = 10
        std::discrete_distribution<int> dist({ 60.0, 30.0, 10.0 });

        std::unordered_set<std::string> usedNames;

        for (int slot = 0; slot < 3; slot++)
        {
            int roll = dist(rng);
            bool placed = false;

            // Try the rolled rarity first; fall back to the other two in order
            for (int attempt = 0; attempt < 3 && !placed; attempt++)
            {
                int b = (roll + attempt) % 3;
                for (Card* c : buckets[b])
                {
                    if (!c || usedNames.count(c->getName())) continue;
                    offeredCards.push_back(c);
                    usedNames.insert(c->getName());
                    placed = true;
                    break;
                }
            }

            if (!placed) break; // pool exhausted
        }
    }

    void BuildUI(std::vector<DrawableObject*>& objectsList)
    {
        ClearUI(objectsList);

        optionAreas.clear();

        GameObject* panel = new GameObject();
        panel->SetSize(1920.0f, 1080.0f);
        panel->SetPosition(glm::vec3(0.0f, 0.0f, 800.0f));
        panel->SetColor(0.0f, 0.0f, 0.0f, 0.82f);
        uiObjects.push_back(panel);
        objectsList.push_back(panel);

        TextObject* title = new TextObject();
        SDL_Color titleColor = { 245, 245, 245, 255 };
        title->LoadText("Choose 1 Card Reward", titleColor, 54);
        title->SetPosition(glm::vec3(0.0f, 420.0f, 810.0f));
        uiObjects.push_back(title);
        objectsList.push_back(title);

        TextObject* hint = new TextObject();
        SDL_Color hintColor = { 220, 220, 220, 255 };
        hint->LoadText("Click a card to claim reward", hintColor, 30);
        hint->SetPosition(glm::vec3(0.0f, 350.0f, 810.0f));
        uiObjects.push_back(hint);
        objectsList.push_back(hint);

        const float cardWidth = 280.0f;
        const float cardHeight = 410.0f;
        const float spacingX = 360.0f;
        const float startX = -0.5f * static_cast<float>(offeredCards.size() - 1) * spacingX;
        const float centerY = 40.0f;

        auto cloneImage = [](ImageObject* src, const glm::vec3& pos, float w, float h) -> ImageObject*
            {
                if (!src)
                {
                    return nullptr;
                }

                ImageObject* copy = new ImageObject();
                copy->SetTextureId(src->GetTextureId());
                copy->SetPosition(pos);
                copy->SetSize(w, h);
                return copy;
            };

        for (int i = 0; i < static_cast<int>(offeredCards.size()); ++i)
        {
            Card* card = offeredCards[i];
            if (!card)
            {
                continue;
            }

            if (!card->HasVisuals())
            {
                card->CreateVisuals();
            }

            glm::vec3 cardPos(startX + (spacingX * i), centerY, 805.0f);

            ImageObject* bg = cloneImage(card->GetBackground(), cardPos, cardWidth, -cardHeight);
            if (bg)
            {
                uiObjects.push_back(bg);
                objectsList.push_back(bg);
            }

            ImageObject* stars = cloneImage(card->GetStarOverlay(), cardPos, cardWidth, -cardHeight);
            if (stars)
            {
                uiObjects.push_back(stars);
                objectsList.push_back(stars);
            }

            ImageObject* type = cloneImage(card->GetTypeIcon(), cardPos, cardWidth, -cardHeight);
            if (type)
            {
                uiObjects.push_back(type);
                objectsList.push_back(type);
            }

            ImageObject* visual = cloneImage(card->GetVisual(), cardPos, cardWidth, -cardHeight);
            if (visual)
            {
                uiObjects.push_back(visual);
                objectsList.push_back(visual);
            }

            ImageObject* frame = cloneImage(card->GetCardFrame(), cardPos, cardWidth, -cardHeight);
            if (frame)
            {
                uiObjects.push_back(frame);
                objectsList.push_back(frame);
            }

            if (card->GetNameText())
            {
                TextObject* src = card->GetNameText();
                TextObject* copy = new TextObject();
                copy->SetTextureId(src->GetTextureId());

                float nameW = src->GetSize().x - 2.5f;
                float nameH = src->GetSize().y - 1.5f;
                if (nameH < 0.0f)
                {
                    nameH += 2.0f;
                }
                else
                {
                    nameH -= 2.0f;
                }

                copy->SetSize(nameW, nameH);
                glm::vec3 local = src->GetLocalPosition();
                float leftAnchorX = (cardPos.x - (cardWidth * 0.5f)) + (local.x * cardWidth);
                float centeredX = leftAnchorX + (copy->GetSize().x * 0.5f) + 12.0f;
                copy->SetPosition(glm::vec3(centeredX, cardPos.y + (local.y * cardHeight), 810.0f));

                uiObjects.push_back(copy);
                objectsList.push_back(copy);
            }

            if (card->GetDescriptionText())
            {
                TextObject* src = card->GetDescriptionText();
                TextObject* copy = new TextObject();
                copy->SetTextureId(src->GetTextureId());
                copy->SetSize(src->GetSize().x, src->GetSize().y);

                glm::vec3 local = src->GetLocalPosition();
                float leftAnchorX = (cardPos.x - (cardWidth * 0.5f)) + (local.x * cardWidth);
                float centeredX = leftAnchorX + (copy->GetSize().x * 0.5f);
                copy->SetPosition(glm::vec3(centeredX, cardPos.y + (local.y * cardHeight), 810.0f));

                uiObjects.push_back(copy);
                objectsList.push_back(copy);
            }

            OptionArea area;
            area.card = card;
            area.minX = cardPos.x - cardWidth * 0.5f;
            area.maxX = cardPos.x + cardWidth * 0.5f;
            area.minY = cardPos.y - cardHeight * 0.5f;
            area.maxY = cardPos.y + cardHeight * 0.5f;
            optionAreas.push_back(area);
        }

        skipButton.Init(
            "../Resource/Texture/UI/SkipBut.png",
            glm::vec3(0.0f, -320.0f, 812.0f),
            glm::vec2(190.0f, -90.0f),
            objectsList
        );
    }

    void ClearUI(std::vector<DrawableObject*>& objectsList)
    {
        for (DrawableObject* obj : uiObjects)
        {
            if (!obj)
            {
                continue;
            }

            auto it = std::find(objectsList.begin(), objectsList.end(), obj);
            if (it != objectsList.end())
            {
                objectsList.erase(it);
            }

            delete obj;
        }

        uiObjects.clear();
        optionAreas.clear();

        auto removeButtonImage = [&](Button& button)
            {
                ImageObject* img = button.GetImage();
                if (!img)
                {
                    button.Reset();
                    return;
                }

                auto it = std::find(objectsList.begin(), objectsList.end(), img);
                if (it != objectsList.end())
                {
                    objectsList.erase(it);
                }

                delete img;
                button.Reset();
            };

        removeButtonImage(skipButton);
    }

    bool IsInside(const OptionArea& area, const glm::vec3& mousePos) const
    {
        return mousePos.x >= area.minX && mousePos.x <= area.maxX && mousePos.y >= area.minY && mousePos.y <= area.maxY;
    }

    Action* CloneAction(const Action* src) const
    {
        if (!src)
        {
            return nullptr;
        }

        Action* copy = nullptr;

        if (dynamic_cast<const AttackAction*>(src))
        {
            const auto* attack = dynamic_cast<const AttackAction*>(src);
            auto* attackCopy = new AttackAction();
            attackCopy->setSubType(attack->getSubType());
            copy = attackCopy;
        }
        else if (const auto* move = dynamic_cast<const MoveAction*>(src))
        {
            copy = new MoveAction(move->getSubType());
        }
        else if (const auto* buff = dynamic_cast<const BuffAction*>(src))
        {
            copy = new BuffAction(buff->getSubType());
        }
        else if (const auto* debuff = dynamic_cast<const DebuffAction*>(src))
        {
            copy = new DebuffAction(debuff->getSubType());
        }
        else if (const auto* energy = dynamic_cast<const EnergyAction*>(src))
        {
            copy = new EnergyAction(energy->getSubType());
        }

        if (!copy)
        {
            return nullptr;
        }

        copy->setActionCode(src->getActionCode());
        copy->setValue(src->getValue());
        copy->setBaseValue(src->getBaseValue());
        copy->setMultiplier(src->getMultiplier());
        copy->setRepeatCount(src->getRepeatCount());
        copy->setRotation(src->getRotation());
        return copy;
    }

    Card* CloneCard(const Card* src) const
    {
        if (!src)
        {
            return nullptr;
        }

        Card* copy = new Card(src->getName());
        copy->setDescription(src->getDescription());
        copy->setLevel(src->getLevel());
        copy->setRarityCode(src->getRarityCode());
        copy->setTypeCode(src->getTypeCode());

        copy->setIsFast(src->getIsFast());
        copy->setIsTemp(src->getIsTemp());
        copy->setIsDeleteAfterUse(src->getIsDeleteAfterUse());
        copy->setIsPersist(src->getIsPersist());
        copy->setIsLag(src->getIsLag());
        copy->setIsPreLoad(src->getIsPreLoad());
        copy->setOverclockValue(src->getOverclockValue());

        for (Action* action : src->getActions())
        {
            Action* actionCopy = CloneAction(action);
            if (actionCopy)
            {
                copy->addAction(actionCopy);
            }
        }

        return copy;
    }

    void GrantReward(Card* card, CardSystem& cardSystem)
    {
        Card* copy = CloneCard(card);
        if (!copy)
        {
            return;
        }

        ownedRewardCards.push_back(copy);
        cardSystem.AddCardToDeck(copy);
    }

public:
    CardRewardSystem()
        : rng(std::random_device{}())
    {
    }

    ~CardRewardSystem()
    {
        for (Card* c : ownedRewardCards)
        {
            if (c)
            {
                delete c;
            }
        }
        ownedRewardCards.clear();

        for (DrawableObject* obj : uiObjects)
        {
            if (obj)
            {
                delete obj;
            }
        }
        uiObjects.clear();
    }

    bool LoadPoolData(const std::string& patternFile, const std::string& cardFile, const std::string& cardDescFile, std::string* outError = nullptr)
    {
        if (!rewardLoader.loadPatternsFromFile(patternFile, outError))
        {
            return false;
        }

        if (!rewardLoader.loadFromFile(cardFile, outError))
        {
            return false;
        }

        if (!cardDescFile.empty())
        {
            if (!rewardLoader.loadActionDescriptionsFromFile(cardDescFile, outError))
            {
                return false;
            }
        }

        rewardPool = rewardLoader.getCards();
        poolLoaded = true;
        return true;
    }

    void ApplyOwnedRewards(CardSystem& cardSystem)
    {
        for (Card* c : ownedRewardCards)
        {
            if (c)
            {
                cardSystem.AddCardToDeck(c);
            }
        }
    }

    void Open(std::vector<DrawableObject*>& objectsList)
    {
        if (active)
        {
            return;
        }

        BuildOffer();
        if (offeredCards.empty())
        {
            return;
        }

        BuildUI(objectsList);
        active = true;
    }

    void Close(std::vector<DrawableObject*>& objectsList)
    {
        if (!active && uiObjects.empty())
        {
            return;
        }

        ClearUI(objectsList);
        offeredCards.clear();
        active = false;
    }

    bool HandleKeySelection(char key, CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
    {
        if (!active)
        {
            return false;
        }

        int index = -1;
        if (key == '1')
        {
            index = 0;
        }
        else if (key == '2')
        {
            index = 1;
        }
        else if (key == '3')
        {
            index = 2;
        }

        if (index < 0 || index >= static_cast<int>(offeredCards.size()))
        {
            return false;
        }

        Card* selected = offeredCards[index];
        if (!selected)
        {
            return false;
        }

        GrantReward(selected, cardSystem);
        Close(objectsList);
        return true;
    }

    bool HandleMouseClick(const glm::vec3& mousePos, CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
    {
        if (!active)
        {
            return false;
        }

        if (skipButton.IsClicked(mousePos.x, mousePos.y))
        {
            Close(objectsList);
            return true;
        }

        for (const OptionArea& area : optionAreas)
        {
            if (!area.card)
            {
                continue;
            }

            if (IsInside(area, mousePos))
            {
                GrantReward(area.card, cardSystem);
                Close(objectsList);
                return true;
            }
        }

        return false;
    }

    bool IsActive() const
    {
        return active;
    }
};
