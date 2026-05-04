// ShopSystem.h
#pragma once

#include <random>
#include <string>
#include <vector>

#include "Button.h"
#include "CardSystem.h"
#include "DrawableObject.h"
#include "GameDataLoader.h"
#include "Player.h"
#include "TextObject.h"

class Action;
class Card;

class ShopSystem
{
private:
    struct ShopSlot
    {
        Card* card = nullptr;
        int price = 0;
        bool sold = false;
        float minX = 0.0f;
        float maxX = 0.0f;
        float minY = 0.0f;
        float maxY= 0.0f;
        TextObject* priceLabel = nullptr;
    };

    GameDataLoader shopLoader;
    std::vector<Card*> shopPool;
    std::vector<Card*> ownedShopCards;
    std::vector<DrawableObject*> uiObjects;
    std::vector<ShopSlot> shopSlots;
    Button closeButton;

    TextObject* coinDisplay = nullptr;
    Player* playerRef = nullptr;

    bool poolLoaded = false;
    bool active = false;

    std::mt19937 rng;

    static constexpr int RARITY_BUCKET_COM = 0;
    static constexpr int RARITY_BUCKET_RAR = 1;
    static constexpr int RARITY_BUCKET_LEG = 2;
    static constexpr int HEAL_PERCENT = 15;

    // Heal service
    bool healEverUsed = false;
    int  healUsesThisShop = 0;
    Button healButton;
    TextObject* healPriceLabel = nullptr;

    // Remove card service
    int  shopVisitCount = 0;
    bool removeUsedThisShop = false;
    Button removeButton;
    TextObject* removePriceLabel = nullptr;
    std::vector<std::string> permanentlyRemovedNames;

    // Remove card overlay (card selection)
    bool selectingCardToRemove = false;
    struct RemoveCardSlot
    {
        Card* card = nullptr;
        float minX = 0, maxX = 0, minY = 0, maxY = 0;
    };
    std::vector<RemoveCardSlot> removeCandidates;
    std::vector<DrawableObject*> removeOverlayObjects;
    std::vector<DrawableObject*> removeCardObjects;
    std::vector<Card*> removeShowCards;
    Button removeCancelButton;
    Button removeLeftButton;
    Button removeRightButton;
    TextObject* removePageLabel = nullptr;
    int removeCurrentPage = 0;
    int removeTotalPages = 0;
    static constexpr int REMOVE_CARDS_PER_PAGE = 5;

    int GetHealCost() const;
    int GetRemoveCost() const;
    void OpenRemoveOverlay(CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList);
    void RebuildRemoveOverlayPage(std::vector<DrawableObject*>& objectsList);
    void CloseRemoveOverlay(std::vector<DrawableObject*>& objectsList);

    int    GeneratePrice(const std::string& rCode);
    void   BuildOffer();
    void   BuildUI(std::vector<DrawableObject*>& objectsList);
    void   ClearUI(std::vector<DrawableObject*>& objectsList);
    Action* CloneAction(const Action* src) const;
    Card*   CloneCard(const Card* src,const GameDataLoader& srcLoader,GameDataLoader& dstLoader) const;

public:
    ShopSystem();
    ~ShopSystem();

    bool LoadPoolData(const std::string& patternFile,const std::string& cardFile,const std::string& cardDescFile,std::string* outError = nullptr);

    void Open(std::vector<DrawableObject*>& objectsList, Player& player);
    void Close(std::vector<DrawableObject*>& objectsList);

    bool HandleMouseClick(const glm::vec3& mousePos,CardSystem& cardSystem,Player& player,std::vector<DrawableObject*>& objectsList);

    void ApplyRemovals(CardSystem& cardSystem);

    bool IsActive() const;
};
