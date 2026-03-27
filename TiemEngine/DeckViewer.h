#pragma once

#include <vector>
#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"
#include "GameObject.h"
#include "TextObject.h"
#include "Button.h"

// One independent visual copy of a card for the deck viewer
struct DeckCardView
{
    vector<DrawableObject*> layers; // All layers we own (image + text copies)
    Card* cardData = nullptr;
    glm::vec3 cardPos = glm::vec3(0.0f);
    float cardW = 0.0f;
    float cardH = 0.0f;
};

class DeckViewer
{
private:
    vector<Card*> deck;
    vector<DeckCardView> cardViews;
    GameObject* background = nullptr;

    int currentPage = 0;
    int cardsPerPage = 10;
    int totalPages = 0;

    bool isActive = false;

    Button leftNavButton;
    Button rightNavButton;
    Button closeButton;

    void createCardVisuals(vector<DrawableObject*>& objectsList);
    void clearCardVisuals(vector<DrawableObject*>& objectsList);
    void createControls(vector<DrawableObject*>& objectsList);
    void clearControls(vector<DrawableObject*>& objectsList);
    ImageObject* cloneImage(ImageObject* src, const glm::vec3& pos, float w, float h);
    bool isPointInsideCard(const glm::vec3& p, const DeckCardView& view) const;

public:
    DeckViewer();
    ~DeckViewer();

    void SetDeck(const vector<Card*>& cards);
    void Show(vector<DrawableObject*>& objectsList);
    void Hide(vector<DrawableObject*>& objectsList);
    bool IsActive() const { return isActive; }

    void NextPage(vector<DrawableObject*>& objectsList);
    void PrevPage(vector<DrawableObject*>& objectsList);
    Card* PeekAt(const glm::vec3& mousePos) const;
    bool HandleClick(const glm::vec3& mousePos, vector<DrawableObject*>& objectsList);

    int GetCurrentPage() const { return currentPage; }
    int GetTotalPages() const { return totalPages; }
};
