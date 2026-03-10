#pragma once

#include <vector>
#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"
#include "TextObject.h"

// One independent visual copy of a card for the deck viewer
struct DeckCardView
{
    vector<DrawableObject*> layers; // All layers we own (image + text copies)
};

class DeckViewer
{
private:
    vector<Card*> deck;
    vector<DeckCardView> cardViews;
    ImageObject* background = nullptr;

    int currentPage = 0;
    int cardsPerPage = 10;
    int totalPages = 0;

    bool isActive = false;

    void createCardVisuals(vector<DrawableObject*>& objectsList);
    void clearCardVisuals(vector<DrawableObject*>& objectsList);
    ImageObject* cloneImage(ImageObject* src, const glm::vec3& pos, float w, float h);

public:
    DeckViewer();
    ~DeckViewer();

    void SetDeck(const vector<Card*>& cards);
    void Show(vector<DrawableObject*>& objectsList);
    void Hide(vector<DrawableObject*>& objectsList);
    bool IsActive() const { return isActive; }

    void NextPage(vector<DrawableObject*>& objectsList);
    void PrevPage(vector<DrawableObject*>& objectsList);

    int GetCurrentPage() const { return currentPage; }
    int GetTotalPages() const { return totalPages; }
};
