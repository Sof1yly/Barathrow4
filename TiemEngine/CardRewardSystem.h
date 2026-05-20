// CardRewardSystem.h

#pragma once

#include <algorithm>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "Action.h"
#include "AttackAction.h"
#include "AttackPattern.h"
#include "BuffAction.h"
#include "CardSystem.h"
#include "DebuffAction.h"
#include "DrawableObject.h"
#include "EnergyAction.h"
#include "GameDataLoader.h"
#include "GameObject.h"
#include "MoveAction.h"
#include "ComboAction.h"
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
        std::vector<DrawableObject*> layers;
    };

    GameDataLoader rewardLoader;
    std::vector<Card*> rewardPool;
    std::vector<Card*> offeredCards;
    std::vector<Card*> ownedRewardCards;
    std::unordered_map<const Action*, const AttackPattern*> clonedActionPatterns;

    std::vector<DrawableObject*> uiObjects;
    std::vector<OptionArea> optionAreas;
    Button skipButton;
    int hoveredCardIdx = -1;

    bool poolLoaded = false;
    bool active = false;
    bool forceLegendary = false;

    std::mt19937 rng;

    // Rarity weights used for the 3 reward slots: com=60%, rar=30%, leg=10%
    static constexpr int RARITY_BUCKET_COM = 0;
    static constexpr int RARITY_BUCKET_RAR = 1;
    static constexpr int RARITY_BUCKET_LEG = 2;

    static void ScaleLayers(std::vector<DrawableObject*>& layers, float cx, float cy, float scale)
    {
        for (DrawableObject* obj : layers) {
            glm::vec3 pos = obj->GetPosition();
            glm::vec2 sz  = obj->GetSize();
            obj->SetPosition(glm::vec3(cx + (pos.x - cx) * scale, cy + (pos.y - cy) * scale, pos.z));
            obj->SetSize(sz.x * scale, sz.y * scale);
        }
    }

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

        // Weighted roll: com = 60, rar = 30, leg = 10  (or 100% legendary for elite/boss)
        std::discrete_distribution<int> dist(
            forceLegendary ? std::initializer_list<double>{ 0.0, 0.0, 100.0 }
                           : std::initializer_list<double>{ 60.0, 30.0, 10.0 });

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

        // ChooseCard banner
        ImageObject* chooseBanner = new ImageObject();
        chooseBanner->SetTexture("../Resource/Texture/UI/RewardScene/CardReward/ChooseCard.PNG");
        chooseBanner->SetSize(960.0f, -167.0f);
        chooseBanner->SetPosition(glm::vec3(0.0f, 420.0f, 810.0f));
        uiObjects.push_back(chooseBanner);
        objectsList.push_back(chooseBanner);

        TextObject* chooseText = new TextObject();
        SDL_Color white = { 245, 245, 245, 255 };
        chooseText->LoadText("Choose a card", white, 40);
        chooseText->SetPosition(glm::vec3(0.0f, 416.0f, 811.0f));
        uiObjects.push_back(chooseText);
        objectsList.push_back(chooseText);

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
                continue;

            if (!card->HasVisuals())
                card->CreateVisuals();

            glm::vec3 cardPos(startX + (spacingX * i), centerY, 805.0f);

            OptionArea area;
            area.card = card;
            area.minX = cardPos.x - cardWidth * 0.5f;
            area.maxX = cardPos.x + cardWidth * 0.5f;
            area.minY = cardPos.y - cardHeight * 0.5f;
            area.maxY = cardPos.y + cardHeight * 0.5f;

            auto trackLayer = [&](DrawableObject* obj) {
                if (obj) {
                    area.layers.push_back(obj);
                    uiObjects.push_back(obj);
                    objectsList.push_back(obj);
                }
            };

            trackLayer(cloneImage(card->GetBackground(),  cardPos, cardWidth, -cardHeight));
            trackLayer(cloneImage(card->GetStarOverlay(), cardPos, cardWidth, -cardHeight));
            trackLayer(cloneImage(card->GetTypeIcon(),    cardPos, cardWidth, -cardHeight));
            trackLayer(cloneImage(card->GetVisual(),      cardPos, cardWidth, -cardHeight));
            trackLayer(cloneImage(card->GetCardFrame(),   cardPos, cardWidth, -cardHeight));

            if (card->GetNameText())
            {
                TextObject* src = card->GetNameText();
                TextObject* copy = new TextObject();
                copy->SetTextureId(src->GetTextureId());

                float nameW = src->GetSize().x - 2.5f;
                float nameH = src->GetSize().y - 1.5f;
                nameH = (nameH < 0.0f) ? nameH + 2.0f : nameH - 2.0f;

                copy->SetSize(nameW, nameH);
                glm::vec3 local = src->GetLocalPosition();
                float leftAnchorX = (cardPos.x - (cardWidth * 0.5f)) + (local.x * cardWidth);
                float centeredX = leftAnchorX + (copy->GetSize().x * 0.5f) + 12.0f;
                copy->SetPosition(glm::vec3(centeredX, cardPos.y + (local.y * cardHeight), 810.0f));
                trackLayer(copy);
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
                trackLayer(copy);
            }

            optionAreas.push_back(std::move(area));
        }

        skipButton.Init(
            "../Resource/Texture/UI/RewardScene/CardReward/Skip.PNG",
            glm::vec3(0.0f, -320.0f, 812.0f),
            glm::vec2(456.0f, -99.0f),
            objectsList
        );

        TextObject* skipText = new TextObject();
        SDL_Color skipColor = { 245, 245, 245, 255 };
        skipText->LoadText("Skip", skipColor, 34);
        skipText->SetPosition(glm::vec3(0.0f, -324.0f, 813.0f));
        uiObjects.push_back(skipText);
        objectsList.push_back(skipText);
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
        hoveredCardIdx = -1;

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
            attackCopy->setPatternId(attack->getPatternId());
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
            auto* debuffCopy = new DebuffAction(debuff->getSubType());
            debuffCopy->setApplyToAll(debuff->getApplyToAll());
            copy = debuffCopy;
        }
        else if (const auto* energy = dynamic_cast<const EnergyAction*>(src))
        {
            copy = new EnergyAction(energy->getSubType());
        }
        else if (const auto* combo = dynamic_cast<const ComboAction*>(src))
        {
            copy = new ComboAction(combo->getTargetCardName());
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

    // srcLoader  – the loader that owns the original card's pattern links
    // dstLoader  – the CardSystem's loader where the clone's patterns must be registered
    Card* CloneCard(const Card* src, const GameDataLoader& srcLoader, GameDataLoader& dstLoader)
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
                // Re-register the pattern in the card system's loader so
                // AttackAction::execute() can find it when the card is played.
                const AttackPattern* pat = srcLoader.getPatternForAction(action);
                if (pat)
                {
                    dstLoader.linkPatternToAction(actionCopy, pat);
                    clonedActionPatterns[actionCopy] = pat;
                }
                copy->addAction(actionCopy);
            }
        }

        return copy;
    }

    void GrantReward(Card* card, CardSystem& cardSystem)
    {
        Card* copy = CloneCard(card,rewardLoader,cardSystem.GetDataLoader());
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

    GameDataLoader& GetRewardLoader() { return rewardLoader; }

    void SetForceLegendary(bool v) { forceLegendary = v; }

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
        GameDataLoader& dstLoader = cardSystem.GetDataLoader();
        for (Card* c : ownedRewardCards)
        {
            if (!c) continue;
            for (Action* action : c->getActions())
            {
                auto it = clonedActionPatterns.find(action);
                if (it != clonedActionPatterns.end() && it->second)
                    dstLoader.linkPatternToAction(action, it->second);
            }
            cardSystem.AddCardToDeck(c);
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

    void Reset()
    {
        for (Card* c : ownedRewardCards)
            if (c) delete c;
        ownedRewardCards.clear();
        clonedActionPatterns.clear();
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

    void HandleHover(float x, float y)
    {
        if (!active) return;

        // Skip button tinting
        ImageObject* img = skipButton.GetImage();
        if (img) {
            if (skipButton.IsClicked(x, y))
                img->SetColor(1.4f, 1.4f, 1.0f);
            else
                img->SetColor(1.0f, 1.0f, 1.0f);
        }

        // Card hover scale
        int newIdx = -1;
        for (int i = 0; i < (int)optionAreas.size(); ++i) {
            const auto& a = optionAreas[i];
            if (x >= a.minX && x <= a.maxX && y >= a.minY && y <= a.maxY) {
                newIdx = i; break;
            }
        }
        if (newIdx == hoveredCardIdx) return;
        if (hoveredCardIdx >= 0 && hoveredCardIdx < (int)optionAreas.size()) {
            auto& a = optionAreas[hoveredCardIdx];
            float cx = (a.minX + a.maxX) * 0.5f;
            float cy = (a.minY + a.maxY) * 0.5f;
            ScaleLayers(a.layers, cx, cy, 1.0f / 1.08f);
        }
        if (newIdx >= 0) {
            auto& a = optionAreas[newIdx];
            float cx = (a.minX + a.maxX) * 0.5f;
            float cy = (a.minY + a.maxY) * 0.5f;
            ScaleLayers(a.layers, cx, cy, 1.08f);
        }
        hoveredCardIdx = newIdx;
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

    Card* PeekCardAt(float x, float y) const
    {
        for (const OptionArea& a : optionAreas)
        {
            if (!a.card) continue;
            if (x >= a.minX && x <= a.maxX && y >= a.minY && y <= a.maxY)
                return a.card;
        }
        return nullptr;
    }

    bool IsActive() const
    {
        return active;
    }
};
