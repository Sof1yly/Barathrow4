#include "ShopSystem.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "Action.h"
#include "AttackAction.h"
#include "AttackPattern.h"
#include "BuffAction.h"
#include "Card.h"
#include "DebuffAction.h"
#include "EnergyAction.h"
#include "GameObject.h"
#include "ImageObject.h"
#include "MoveAction.h"


ShopSystem::ShopSystem(): rng(std::random_device{}())
{
}

ShopSystem::~ShopSystem()
{
    for (Card* c : ownedShopCards) {
        if (c) delete c;
        ownedShopCards.clear();
    }


    for (DrawableObject* obj : uiObjects) {
        if (obj) delete obj;
        uiObjects.clear();
    }

}



int ShopSystem::GeneratePrice(const std::string& rCode)
{
    if (rCode == "com") return std::uniform_int_distribution<int>(90,  100)(rng);
    if (rCode == "rar") return std::uniform_int_distribution<int>(130, 150)(rng);
    if (rCode == "leg") return std::uniform_int_distribution<int>(190, 210)(rng);
    return 100;
}

void ShopSystem::BuildOffer()
{
    shopSlots.clear();

    if (!poolLoaded) {
        return;
    }
       

    std::vector<Card*> buckets[3];
    for (Card* c : shopPool)
    {
        if (!c || c->isEnergyCard()) continue;
        const std::string& r = c->getRarityCode();
        if (r == "com") {
            buckets[RARITY_BUCKET_COM].push_back(c);
        }
        else if (r == "rar") {
            buckets[RARITY_BUCKET_RAR].push_back(c);
        }
        else if (r == "leg") {
            buckets[RARITY_BUCKET_LEG].push_back(c);
        }
    }

    for (auto& b : buckets) {
        std::shuffle(b.begin(), b.end(), rng);
    }
       

    std::discrete_distribution<int> dist({ 60.0, 30.0, 10.0 });
    std::unordered_set<std::string> usedNames;

    for (int slot = 0; slot < 5; slot++)
    {
        int  roll   = dist(rng);
        bool placed = false;

        for (int attempt = 0; attempt < 3 && !placed; attempt++)
        {
            int b = (roll + attempt) % 3;
            for (Card* c : buckets[b])
            {
                if (!c || usedNames.count(c->getName())) continue;

                ShopSlot s;
                s.card  = c;
                s.price = GeneratePrice(c->getRarityCode());
                shopSlots.push_back(s);
                usedNames.insert(c->getName());
                placed = true;
                break;
            }
        }

        if (!placed) break;
    }
}

void ShopSystem::BuildUI(std::vector<DrawableObject*>& objectsList)
{
    ClearUI(objectsList);

    // Semi-transparent dark panel
    GameObject* panel = new GameObject();
    panel->SetSize(1920.0f, 1080.0f);
    panel->SetPosition(glm::vec3(0.0f, 0.0f, 800.0f));
    panel->SetColor(0.0f, 0.0f, 0.0f, 0.85f);
    uiObjects.push_back(panel);
    objectsList.push_back(panel);

    // Title
    TextObject* title = new TextObject();
    SDL_Color titleColor = { 255, 215, 90, 255 };
    title->LoadText("Shop", titleColor, 58);
    title->SetPosition(glm::vec3(0.0f, 430.0f, 810.0f));
    uiObjects.push_back(title);
    objectsList.push_back(title);

    // Sub-hint
    TextObject* hint = new TextObject();
    SDL_Color hintColor = { 200, 200, 200, 255 };
    hint->LoadText("Click a card to purchase it", hintColor, 26);
    hint->SetPosition(glm::vec3(0.0f, 368.0f, 810.0f));
    uiObjects.push_back(hint);
    objectsList.push_back(hint);

    // Live coin counter
    coinDisplay = new TextObject();
    SDL_Color coinColor = { 255, 210, 40, 255 };
    int startCoins = playerRef ? playerRef->GetCoins() : 0;
    coinDisplay->LoadText("Coins: " + std::to_string(startCoins), coinColor, 30);
    coinDisplay->SetPosition(glm::vec3(0.0f, 312.0f, 810.0f));
    uiObjects.push_back(coinDisplay);
    objectsList.push_back(coinDisplay);

    const float cardWidth  = 270.0f;
    const float cardHeight = 400.0f;
    const float spacingX   = 320.0f;
    const int   count      = static_cast<int>(shopSlots.size());
    const float startX     = -0.5f * static_cast<float>(count - 1) * spacingX;
    const float centerY    = 20.0f;

    auto cloneImage = [](ImageObject* src, const glm::vec3& pos, float w, float h) -> ImageObject*
    {
        if (!src) return nullptr;
        ImageObject* copy = new ImageObject();
        copy->SetTextureId(src->GetTextureId());
        copy->SetPosition(pos);
        copy->SetSize(w, h);
        return copy;
    };

    auto pushObj = [&](DrawableObject* obj)
    {
        if (obj)
        {
            uiObjects.push_back(obj);
            objectsList.push_back(obj);
        }
    };

    for (int i = 0; i < count; ++i)
    {
        ShopSlot& slot = shopSlots[i];
        Card* card = slot.card;
        if (!card) continue;

        if (!card->HasVisuals())
            card->CreateVisuals();

        glm::vec3 cardPos(startX + spacingX * i, centerY, 805.0f);

        // Clone card visual layers
        pushObj(cloneImage(card->GetBackground(),cardPos, cardWidth, -cardHeight));
        pushObj(cloneImage(card->GetStarOverlay(),cardPos, cardWidth, -cardHeight));
        pushObj(cloneImage(card->GetTypeIcon(), cardPos, cardWidth, -cardHeight));
        pushObj(cloneImage(card->GetVisual(),cardPos, cardWidth, -cardHeight));
        pushObj(cloneImage(card->GetCardFrame(),cardPos, cardWidth, -cardHeight));

        // Name text
        if (card->GetNameText())
        {
            TextObject* src  = card->GetNameText();
            TextObject* copy = new TextObject();
            copy->SetTextureId(src->GetTextureId());

            float nameW = src->GetSize().x - 2.5f;
            float nameH = src->GetSize().y - 1.5f;
            nameH = (nameH < 0.0f) ? nameH + 2.0f : nameH - 2.0f;

            copy->SetSize(nameW, nameH);
            glm::vec3 local   = src->GetLocalPosition();
            float leftAnchorX = (cardPos.x - cardWidth * 0.5f) + (local.x * cardWidth);
            float centeredX   = leftAnchorX + copy->GetSize().x * 0.5f + 12.0f;
            copy->SetPosition(glm::vec3(centeredX, cardPos.y + local.y * cardHeight, 810.0f));
            uiObjects.push_back(copy);
            objectsList.push_back(copy);
        }

        // Description text
        if (card->GetDescriptionText())
        {
            TextObject* src  = card->GetDescriptionText();
            TextObject* copy = new TextObject();
            copy->SetTextureId(src->GetTextureId());
            copy->SetSize(src->GetSize().x, src->GetSize().y);

            glm::vec3 local   = src->GetLocalPosition();
            float leftAnchorX = (cardPos.x - cardWidth * 0.5f) + (local.x * cardWidth);
            float centeredX   = leftAnchorX + copy->GetSize().x * 0.5f;
            copy->SetPosition(glm::vec3(centeredX, cardPos.y + local.y * cardHeight, 810.0f));
            uiObjects.push_back(copy);
            objectsList.push_back(copy);
        }

        // Price label below card
        {
            SDL_Color priceColor = { 255, 210, 40, 255 };
            TextObject* priceText = new TextObject();
            priceText->LoadText(std::to_string(slot.price) + " coins", priceColor, 24);
            priceText->SetPosition(glm::vec3(cardPos.x,cardPos.y - cardHeight * 0.5f - 30.0f,812.0f));
            uiObjects.push_back(priceText);
            objectsList.push_back(priceText);
            slot.priceLabel = priceText;
        }

        // Click area
        slot.minX = cardPos.x - cardWidth  * 0.5f;
        slot.maxX = cardPos.x + cardWidth  * 0.5f;
        slot.minY = cardPos.y - cardHeight * 0.5f;
        slot.maxY = cardPos.y + cardHeight * 0.5f;
    }

    // Close / Leave button (center bottom)
    closeButton.Init("../Resource/Texture/UI/SkipBut.png", glm::vec3(0.0f, -370.0f, 812.0f), glm::vec2(190.0f, -90.0f), objectsList);

    SDL_Color goldColor = { 255, 210, 40, 255 };

    // Heal button (right bottom)
    healButton.Init("../Resource/Texture/UI/SkipBut.png", glm::vec3(350.0f, -370.0f, 812.0f), glm::vec2(190.0f, -90.0f), objectsList);
    healPriceLabel = new TextObject();
    {
        int hCost = GetHealCost();
        std::string hStr = (hCost == 0) ? "Heal: FREE" : ("Heal: " + std::to_string(hCost) + "g");
        healPriceLabel->LoadText(hStr, goldColor, 22);
    }
    healPriceLabel->SetPosition(glm::vec3(350.0f, -310.0f, 812.0f));
    uiObjects.push_back(healPriceLabel);
    objectsList.push_back(healPriceLabel);

    // Remove card button (left bottom)
    removeButton.Init("../Resource/Texture/UI/cross.png", glm::vec3(-350.0f, -370.0f, 812.0f), glm::vec2(190.0f, -90.0f), objectsList);
    removePriceLabel = new TextObject();
    {
        std::string rStr = removeUsedThisShop
            ? "Remove: USED"
            : ("Remove: " + std::to_string(GetRemoveCost()) + "g");
        SDL_Color rColor = removeUsedThisShop
            ? SDL_Color{ 160, 160, 160, 200 }
            : goldColor;
        removePriceLabel->LoadText(rStr, rColor, 22);
    }
    removePriceLabel->SetPosition(glm::vec3(-350.0f, -310.0f, 812.0f));
    uiObjects.push_back(removePriceLabel);
    objectsList.push_back(removePriceLabel);
}

void ShopSystem::ClearUI(std::vector<DrawableObject*>& objectsList)
{
    if (selectingCardToRemove)
        CloseRemoveOverlay(objectsList);

    for (DrawableObject* obj : uiObjects)
    {
        if (!obj) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    uiObjects.clear();
    coinDisplay     = nullptr;
    healPriceLabel  = nullptr;
    removePriceLabel = nullptr;

    for (ShopSlot& s : shopSlots)
        s.priceLabel = nullptr;

    auto removeButtonImage = [&](Button& button)
    {
        ImageObject* img = button.GetImage();
        if (!img) { button.Reset(); return; }
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) objectsList.erase(it);
        delete img;
        button.Reset();
    };
    removeButtonImage(closeButton);
    removeButtonImage(healButton);
    removeButtonImage(removeButton);
}

Action* ShopSystem::CloneAction(const Action* src) const
{
    if (!src) return nullptr;
    Action* copy = nullptr;

    if (const auto* atk = dynamic_cast<const AttackAction*>(src))
    {
        auto* ac = new AttackAction();
        ac->setSubType(atk->getSubType());
        copy = ac;
    }
    else if (const auto* mv = dynamic_cast<const MoveAction*>(src)) {
        copy = new MoveAction(mv->getSubType());
    }  
    else if (const auto* bf = dynamic_cast<const BuffAction*>(src)) {
        copy = new BuffAction(bf->getSubType());
    }  
    else if (const auto* db = dynamic_cast<const DebuffAction*>(src)) {
        copy = new DebuffAction(db->getSubType());
    }  
    else if (const auto* en = dynamic_cast<const EnergyAction*>(src)) {
        copy = new EnergyAction(en->getSubType());
    }

    if (!copy) return nullptr;

    copy->setActionCode(src->getActionCode());
    copy->setValue(src->getValue());
    copy->setBaseValue(src->getBaseValue());
    copy->setMultiplier(src->getMultiplier());
    copy->setRepeatCount(src->getRepeatCount());
    copy->setRotation(src->getRotation());
    return copy;
}

Card* ShopSystem::CloneCard(const Card* src, const GameDataLoader& srcLoader,GameDataLoader& dstLoader) const
{
    if (!src) return nullptr;

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
        Action* ac = CloneAction(action);
        if (ac)
        {
            // Re-register the attack pattern in the card system's loader so
            // AttackAction::execute() can find it when the cloned card is played.
            const AttackPattern* pat = srcLoader.getPatternForAction(action);
            if (pat)
            {
                dstLoader.linkPatternToAction(ac, pat);
            }
            copy->addAction(ac);
        }
    }
    return copy;
}



int ShopSystem::GetHealCost() const
{
    if (!healEverUsed) return 0;
    return 50 + 25 * healUsesThisShop;
}

int ShopSystem::GetRemoveCost() const
{
    return 100 + 25 * (shopVisitCount - 1);
}

void ShopSystem::OpenRemoveOverlay(CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
{
    std::vector<Card*> allCards = cardSystem.GetAllCards();
    std::unordered_set<std::string> seenNames;
    removeShowCards.clear();

    for (Card* c : allCards)
    {
        if (!c || c->isEnergyCard()) continue;
        if (seenNames.count(c->getName())) continue;
        bool wasRemoved = false;
        for (const auto& n : permanentlyRemovedNames)
            if (n == c->getName()) { wasRemoved = true; break; }
        if (wasRemoved) continue;
        seenNames.insert(c->getName());
        removeShowCards.push_back(c);
    }

    if (removeShowCards.empty()) { std::cout << "[Shop] No cards to remove.\n"; return; }

    selectingCardToRemove = true;
    removeCurrentPage = 0;
    removeTotalPages = ((int)removeShowCards.size() + REMOVE_CARDS_PER_PAGE - 1) / REMOVE_CARDS_PER_PAGE;
    removeCandidates.clear();

    auto pushStatic = [&](DrawableObject* obj)
    {
        if (obj) { removeOverlayObjects.push_back(obj); objectsList.push_back(obj); }
    };

    // Dim overlay
    GameObject* bg = new GameObject();
    bg->SetSize(1920.0f, 1080.0f);
    bg->SetPosition(glm::vec3(0.0f, 0.0f, 815.0f));
    bg->SetColor(0.0f, 0.0f, 0.0f, 0.96f);
    pushStatic(bg);

    // Title
    TextObject* title = new TextObject();
    SDL_Color titleColor = { 255, 215, 90, 255 };
    title->LoadText("Select a card to remove", titleColor, 40);
    title->SetPosition(glm::vec3(0.0f, 420.0f, 820.0f));
    pushStatic(title);

    // Cost label
    TextObject* costLabel = new TextObject();
    SDL_Color goldColor = { 255, 210, 40, 255 };
    costLabel->LoadText("Cost: " + std::to_string(GetRemoveCost()) + " coins", goldColor, 26);
    costLabel->SetPosition(glm::vec3(0.0f, 365.0f, 820.0f));
    pushStatic(costLabel);

    // Page indicator (placeholder, rebuilt each page)
    removePageLabel = new TextObject();
    SDL_Color grayColor = { 200, 200, 200, 255 };
    removePageLabel->LoadText("Page 1 / 1", grayColor, 22);
    removePageLabel->SetPosition(glm::vec3(0.0f, 315.0f, 820.0f));
    pushStatic(removePageLabel);

    // Nav buttons
    removeLeftButton.Init("../Resource/Texture/UI/Left.png",
                          glm::vec3(-870.0f, 20.0f, 820.0f),
                          glm::vec2(80.0f, -80.0f), objectsList);
    removeRightButton.Init("../Resource/Texture/UI/Right.png",
                           glm::vec3(870.0f, 20.0f, 820.0f),
                           glm::vec2(80.0f, -80.0f), objectsList);

    // Cancel button
    removeCancelButton.Init("../Resource/Texture/UI/SkipBut.png",
                            glm::vec3(0.0f, -400.0f, 820.0f),
                            glm::vec2(190.0f, -90.0f), objectsList);

    TextObject* cancelLabel = new TextObject();
    cancelLabel->LoadText("Cancel", grayColor, 26);
    cancelLabel->SetPosition(glm::vec3(0.0f, -335.0f, 820.0f));
    pushStatic(cancelLabel);

    RebuildRemoveOverlayPage(objectsList);
}

void ShopSystem::RebuildRemoveOverlayPage(std::vector<DrawableObject*>& objectsList)
{
    // Clear previous page's card visuals
    for (DrawableObject* obj : removeCardObjects)
    {
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    removeCardObjects.clear();
    removeCandidates.clear();

    // Update page label
    if (removePageLabel)
    {
        SDL_Color grayColor = { 200, 200, 200, 255 };
        removePageLabel->LoadText(
            "Page " + std::to_string(removeCurrentPage + 1) + " / " + std::to_string(removeTotalPages),
            grayColor, 22);
    }

    auto pushCard = [&](DrawableObject* obj)
    {
        if (obj) { removeCardObjects.push_back(obj); objectsList.push_back(obj); }
    };

    auto cloneImage = [](ImageObject* src, const glm::vec3& pos, float w, float h) -> ImageObject*
    {
        if (!src) return nullptr;
        ImageObject* copy = new ImageObject();
        copy->SetTextureId(src->GetTextureId());
        copy->SetPosition(pos);
        copy->SetSize(w, h);
        return copy;
    };

    const float cardW   = 280.0f;
    const float cardH   = 410.0f;
    const float spacing = 320.0f;
    const float centerY = 20.0f;

    int startIdx = removeCurrentPage * REMOVE_CARDS_PER_PAGE;
    int endIdx   = std::min(startIdx + REMOVE_CARDS_PER_PAGE, (int)removeShowCards.size());
    int count    = endIdx - startIdx;

    float startX = -0.5f * static_cast<float>(count - 1) * spacing;

    for (int i = 0; i < count; ++i)
    {
        Card* card = removeShowCards[startIdx + i];
        if (!card) continue;
        if (!card->HasVisuals()) card->CreateVisuals();

        float cardX = startX + i * spacing;
        glm::vec3 cardPos(cardX, centerY, 818.0f);

        pushCard(cloneImage(card->GetBackground(),  cardPos, cardW, -cardH));
        pushCard(cloneImage(card->GetStarOverlay(), cardPos, cardW, -cardH));
        pushCard(cloneImage(card->GetTypeIcon(),    cardPos, cardW, -cardH));
        pushCard(cloneImage(card->GetVisual(),      cardPos, cardW, -cardH));
        pushCard(cloneImage(card->GetCardFrame(),   cardPos, cardW, -cardH));

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
            float leftX = (cardX - cardW * 0.5f) + (local.x * cardW);
            float centX = leftX + copy->GetSize().x * 0.5f + 12.0f;
            copy->SetPosition(glm::vec3(centX, centerY + local.y * cardH, 822.0f));
            pushCard(copy);
        }

        if (card->GetDescriptionText())
        {
            TextObject* src  = card->GetDescriptionText();
            TextObject* copy = new TextObject();
            copy->SetTextureId(src->GetTextureId());
            copy->SetSize(src->GetSize().x, src->GetSize().y);
            glm::vec3 local = src->GetLocalPosition();
            float leftX = (cardX - cardW * 0.5f) + (local.x * cardW);
            float centX = leftX + copy->GetSize().x * 0.5f;
            copy->SetPosition(glm::vec3(centX, centerY + local.y * cardH, 822.0f));
            pushCard(copy);
        }

        RemoveCardSlot slot;
        slot.card = card;
        slot.minX = cardX - cardW * 0.5f;
        slot.maxX = cardX + cardW * 0.5f;
        slot.minY = centerY - cardH * 0.5f;
        slot.maxY = centerY + cardH * 0.5f;
        removeCandidates.push_back(slot);
    }
}

void ShopSystem::CloseRemoveOverlay(std::vector<DrawableObject*>& objectsList)
{
    // Remove page card visuals
    for (DrawableObject* obj : removeCardObjects)
    {
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    removeCardObjects.clear();
    removeCandidates.clear();
    removeShowCards.clear();
    removePageLabel = nullptr;

    // Remove static overlay objects
    for (DrawableObject* obj : removeOverlayObjects)
    {
        if (!obj) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    removeOverlayObjects.clear();

    auto removeBtn = [&](Button& button)
    {
        ImageObject* img = button.GetImage();
        if (!img) { button.Reset(); return; }
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) objectsList.erase(it);
        delete img;
        button.Reset();
    };
    removeBtn(removeCancelButton);
    removeBtn(removeLeftButton);
    removeBtn(removeRightButton);

    selectingCardToRemove = false;
}

void ShopSystem::ApplyRemovals(CardSystem& cardSystem)
{
    for (const std::string& name : permanentlyRemovedNames)
        cardSystem.RemoveCardEverywhere(name);
}

bool ShopSystem::LoadPoolData(const std::string& patternFile,const std::string& cardFile,const std::string& cardDescFile,std::string* outError)
{
    if (!shopLoader.loadPatternsFromFile(patternFile, outError)) return false;
    if (!shopLoader.loadFromFile(cardFile, outError)) return false;
    if (!cardDescFile.empty())
    if (!shopLoader.loadActionDescriptionsFromFile(cardDescFile, outError)) return false;

    shopPool   = shopLoader.getCards();
    poolLoaded = true;
    return true;
}

void ShopSystem::Open(std::vector<DrawableObject*>& objectsList, Player& player)
{
    if (active) return;

    shopVisitCount++;
    healUsesThisShop = 0;
    removeUsedThisShop = false;
    selectingCardToRemove = false;

    playerRef = &player;
    BuildOffer();
    if (shopSlots.empty()) return;

    BuildUI(objectsList);
    active = true;
}

void ShopSystem::Close(std::vector<DrawableObject*>& objectsList)
{
    if (!active && uiObjects.empty()) return;

    ClearUI(objectsList);
    shopSlots.clear();
    active = false;
}

bool ShopSystem::HandleMouseClick(const glm::vec3& mousePos,CardSystem& cardSystem,Player& player,std::vector<DrawableObject*>& objectsList)
{
    if (!active) return false;

    // Remove card overlay takes priority
    if (selectingCardToRemove)
    {
        if (removeCancelButton.IsClicked(mousePos.x, mousePos.y))
        {
            CloseRemoveOverlay(objectsList);
            return true;
        }

        if (removeLeftButton.IsClicked(mousePos.x, mousePos.y))
        {
            if (removeCurrentPage > 0)
            {
                removeCurrentPage--;
                RebuildRemoveOverlayPage(objectsList);
            }
            return true;
        }

        if (removeRightButton.IsClicked(mousePos.x, mousePos.y))
        {
            if (removeCurrentPage < removeTotalPages - 1)
            {
                removeCurrentPage++;
                RebuildRemoveOverlayPage(objectsList);
            }
            return true;
        }

        for (auto& slot : removeCandidates)
        {
            if (mousePos.x >= slot.minX && mousePos.x <= slot.maxX &&
                mousePos.y >= slot.minY && mousePos.y <= slot.maxY)
            {
                if (!slot.card) continue;
                int cost = GetRemoveCost();
                if (player.GetCoins() < cost)
                {
                    std::cout << "[Shop] Not enough coins to remove! Need " << cost << "\n";
                    CloseRemoveOverlay(objectsList);
                    return true;
                }
                player.SpendCoins(cost);
                permanentlyRemovedNames.push_back(slot.card->getName());
                cardSystem.RemoveCardEverywhere(slot.card->getName());
                removeUsedThisShop = true;
                std::cout << "[Shop] Removed '" << slot.card->getName() << "' for " << cost << " coins.\n";
                CloseRemoveOverlay(objectsList);
                if (removePriceLabel)
                {
                    SDL_Color gray = { 160, 160, 160, 200 };
                    removePriceLabel->LoadText("Remove: USED", gray, 22);
                }
                if (coinDisplay)
                {
                    SDL_Color coinColor = { 255, 210, 40, 255 };
                    coinDisplay->LoadText("Coins: " + std::to_string(player.GetCoins()), coinColor, 30);
                }
                return true;
            }
        }
        return true; // consume all clicks while overlay is open
    }

    if (closeButton.IsClicked(mousePos.x, mousePos.y))
    {
        Close(objectsList);
        return true;
    }

    // Heal button
    if (healButton.IsClicked(mousePos.x, mousePos.y))
    {
        int cost = GetHealCost();
        if (cost > 0 && player.GetCoins() < cost)
        {
            std::cout << "[Shop] Not enough coins to heal! Need " << cost << "\n";
            return true;
        }
        int healAmount = std::max(1, player.getMaxHp() * HEAL_PERCENT / 100);
        if (cost > 0) player.SpendCoins(cost);
        player.HealHp(healAmount);
        healEverUsed = true;
        healUsesThisShop++;
        if (healPriceLabel)
        {
            SDL_Color goldColor = { 255, 210, 40, 255 };
            int nextCost = GetHealCost();
            healPriceLabel->LoadText("Heal: " + std::to_string(nextCost) + "g", goldColor, 22);
        }
        if (coinDisplay)
        {
            SDL_Color coinColor = { 255, 210, 40, 255 };
            coinDisplay->LoadText("Coins: " + std::to_string(player.GetCoins()), coinColor, 30);
        }
        std::cout << "[Shop] Healed " << healAmount << " HP. Cost: " << cost << " coins.\n";
        return true;
    }

    // Remove card button
    if (removeButton.IsClicked(mousePos.x, mousePos.y))
    {
        if (removeUsedThisShop)
        {
            std::cout << "[Shop] Remove already used this visit.\n";
            return true;
        }
        OpenRemoveOverlay(cardSystem, objectsList);
        return true;
    }

    for (ShopSlot& slot : shopSlots)
    {
        if (slot.sold || !slot.card) continue;

        if (mousePos.x >= slot.minX && mousePos.x <= slot.maxX &&
            mousePos.y >= slot.minY && mousePos.y <= slot.maxY)
        {
            if (player.GetCoins() < slot.price)
            {
                std::cout << "[Shop] Not enough coins! Need "<< slot.price << ", have " << player.GetCoins() << "\n";
                return true;
            }

            player.SpendCoins(slot.price);
            std::cout << "[Shop] Bought '" << slot.card->getName()<< "' for " << slot.price<< " coins. Remaining: " << player.GetCoins() << "\n";

            Card* copy = CloneCard(slot.card,
                                   shopLoader,
                                   cardSystem.GetDataLoader());
            if (copy)
            {
                ownedShopCards.push_back(copy);
                cardSystem.AddCardToDeck(copy);
            }

            slot.sold = true;

            if (slot.priceLabel)
            {
                SDL_Color soldColor = { 160, 160, 160, 200 };
                slot.priceLabel->LoadText("SOLD", soldColor, 24);
            }

            if (coinDisplay)
            {
                SDL_Color coinColor = { 255, 210, 40, 255 };
                coinDisplay->LoadText("Coins: " + std::to_string(player.GetCoins()), coinColor, 30);
            }

            return true;
        }
    }

    return false;
}

bool ShopSystem::IsActive() const
{
    return active;
}
