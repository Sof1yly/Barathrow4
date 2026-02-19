#pragma once

#include <vector>
#include <string>

#include "GameEngine.h"
#include "GameObject.h"
#include "ImageObject.h"
#include "Hand.h"
#include "Card.h"
#include "GameDataLoader.h"

#include <unordered_map>

class TextObject;

class CardSystem
{
private:
    //Data
    GameDataLoader dataLoader;
    Hand hand;

    // Piles
    std::vector<Card*> deck;
    std::vector<Card*> discard;
    std::vector<Card*> deletePile;

    // Track total overclock bonus applied
    int overclockBonus = 0;

    // Pile UI buttons
    ImageObject* drawPileButton = nullptr;
    ImageObject* discardPileButton = nullptr;

    // Pile background card images (rendered behind buttons)
    ImageObject* drawPileBG = nullptr;
    ImageObject* discardPileBG = nullptr;
    glm::vec3 drawPileBGPos = glm::vec3(0.0f);
    glm::vec3 discardPileBGPos = glm::vec3(0.0f);

    // Draw pile turn counter
    int drawPileTurns = 4;
    static const int DRAW_PILE_MAX_TURNS = 4;
    TextObject* drawPileTurnText = nullptr;

    // Drop zones (LEFT=0, TOP=1, BOTTOM=2, RIGHT=3)
    GameObject* dropZones[4] = { nullptr, nullptr, nullptr, nullptr };
    bool dropZonesCreated = false;
    bool dropZonesVisible = false;
    glm::vec3   dropZoneSavedPos[4];

    // Dragging
    bool isDragging = false;
    bool isHolding = false;
    ImageObject* draggingCard = nullptr;
    ImageObject* pendingCard = nullptr;
    glm::vec3 dragStartPos = glm::vec3(0.0f);
    glm::vec3 dragMouseWorld = glm::vec3(0.0f);
    glm::vec3 dragAnchor = glm::vec3(0.0f);
    std::unordered_map<DrawableObject*, glm::vec3> dragLayerOffsets;

    // Bezier leash
    static const int BEZIER_SEGMENTS = 32;
    std::vector<GameObject*> bezierSegments;
    bool  bezierCreated = false;
    float screenCenterY = 0.0f;

    // Internal helpers
    void CreateDropZones(std::vector<DrawableObject*>& list);
    void EnsureBezierSegments(std::vector<DrawableObject*>& list);
    void UpdateBezier(const glm::vec3& P0, const glm::vec3& P1);
    void HideBezier();
    bool IsPointInsideZone(const glm::vec3& p, DrawableObject* zone) const;
    void UpdatePileVisuals();
    void UpdateDrawPileTurnText();

public:
    CardSystem();
    ~CardSystem();

    // Initialization
    bool LoadData(const std::string& patternFile, const std::string& cardFile, std::string* outError = nullptr);
    void InitUI(std::vector<DrawableObject*>& objectsList);

    // Deck management
    void ShuffleDeck();
    void DealNewHand(int cardCount, std::vector<DrawableObject*>& objectsList);
    void DiscardHandAndDraw(int cardCount, std::vector<DrawableObject*>& objectsList);
    void DiscardCard(Card* card);

    // Drop zones
    void ShowDropZones();
    void HideDropZones();
    int  HitDropZone(const glm::vec3& p) const;

    // Dragging
    void BeginDrag(ImageObject* card, const glm::vec3& mouseWorld, std::vector<DrawableObject*>& objectsList);
    void UpdateDrag(const glm::vec3& mouseWorld);
    void EndDragCancel(const glm::vec3& mouseWorld, std::vector<DrawableObject*>& objectsList);
    void EndDragConfirm(ImageObject* card, std::vector<DrawableObject*>& objectsList);
    void EndDragConfirmDelete(ImageObject* card, std::vector<DrawableObject*>& objectsList);

    // Hand queries / hover
    void UpdateHover(const glm::vec3& mousePos, bool dragging, std::vector<DrawableObject*>& objectsList);
    ImageObject* PeekAt(const glm::vec3& mousePos);
    Card* FindCardByImage(ImageObject* img);
    std::vector<DrawableObject*> GetAllLayersForCard(ImageObject* card);
    void RemoveCardView(ImageObject* card, std::vector<DrawableObject*>& objectsList);

    // Button hit tests
    bool IsDrawPileClicked(const glm::vec3& mousePos) const;
    bool IsDiscardPileClicked(const glm::vec3& mousePos) const;

    // Debug info
    void PrintDiscardPile() const;
    void PrintDrawDeck() const;

    // Cleanup
    void Clear(std::vector<DrawableObject*>& objectsList);
    void Reset(std::vector<DrawableObject*>& objectsList);

    // Getters
    bool IsDragging() const { return isDragging; }

    ImageObject* GetDraggingCard() const { return draggingCard; }
    ImageObject* GetPendingCard() const { return pendingCard; }

    void SetPendingCard(ImageObject* card) { pendingCard = card; }

    glm::vec3 GetDragStartPos() const { return dragStartPos; }

    const GameDataLoader& GetDataLoader() const { return dataLoader; }
    Hand& GetHand() { return hand; }

    const std::vector<Card*>& GetDeck() const { return deck; }
    const std::vector<Card*>& GetDiscard() const { return discard; }

    // Draw pile turn counter
    bool UseDrawPileTurn();       // returns true if turn should end (turns was > 0)
    void ResetDrawPileTurns();    // resets counter back to max
    void DecrementDrawPileTurn(); //  counter when player ends turn by playing a card
    int  GetDrawPileTurns() const { return drawPileTurns; }

    // Overclock
    void ApplyOverclock(int amount);
    void ResetOverclock();
    int  GetOverclockBonus() const { return overclockBonus; }
};
