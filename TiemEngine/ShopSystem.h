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
    Player*playerRef   = nullptr;

    bool poolLoaded = false;
    bool active = false;

    std::mt19937 rng;

    static constexpr int RARITY_BUCKET_COM = 0;
    static constexpr int RARITY_BUCKET_RAR = 1;
    static constexpr int RARITY_BUCKET_LEG = 2;

    int    GeneratePrice(const std::string& rCode);
    void   BuildOffer();
    void   BuildUI(std::vector<DrawableObject*>& objectsList);
    void   ClearUI(std::vector<DrawableObject*>& objectsList);
    Action* CloneAction(const Action* src) const;
    Card*   CloneCard(const Card* src) const;

public:
    ShopSystem();
    ~ShopSystem();

    bool LoadPoolData(const std::string& patternFile,const std::string& cardFile,const std::string& cardDescFile,std::string* outError = nullptr);

    void Open(std::vector<DrawableObject*>& objectsList, Player& player);
    void Close(std::vector<DrawableObject*>& objectsList);

    bool HandleMouseClick(const glm::vec3& mousePos,CardSystem& cardSystem,Player& player,std::vector<DrawableObject*>& objectsList);

    bool IsActive() const;
};
