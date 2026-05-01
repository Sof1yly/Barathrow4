#include "ShopSystem.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "Action.h"
#include "AttackAction.h"
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

    // Close / Leave button
    closeButton.Init("../Resource/Texture/UI/SkipBut.png",glm::vec3(0.0f, -370.0f, 812.0f),glm::vec2(190.0f, -90.0f),objectsList);
}

void ShopSystem::ClearUI(std::vector<DrawableObject*>& objectsList)
{
    for (DrawableObject* obj : uiObjects)
    {
        if (!obj) continue;
        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) objectsList.erase(it);
        delete obj;
    }
    uiObjects.clear();
    coinDisplay = nullptr;

    for (ShopSlot& s : shopSlots) {
        s.priceLabel = nullptr;
    }
        

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

Card* ShopSystem::CloneCard(const Card* src) const
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
        if (ac) copy->addAction(ac);
    }
    return copy;
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

    if (closeButton.IsClicked(mousePos.x, mousePos.y))
    {
        Close(objectsList);
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

            Card* copy = CloneCard(slot.card);
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
