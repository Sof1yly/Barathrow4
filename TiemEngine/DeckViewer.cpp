#include "DeckViewer.h"
#include "GameEngine.h"
#include <cmath>

DeckViewer::DeckViewer()
{
    background = new ImageObject();
    background->SetSize(1280, 720);
    background->SetPosition(glm::vec3(0, 0, 0));
    background->SetAlpha(0.7f);
}

DeckViewer::~DeckViewer()
{
    if (background != nullptr)
    {
        delete background;
        background = nullptr;
    }

    for (auto& view : cardViews)
    {
        for (auto* layer : view.layers)
            delete layer;
    }
    cardViews.clear();
}

void DeckViewer::SetDeck(const vector<Card*>& cards)
{
    deck = cards;
    totalPages = (int)ceil((float)deck.size() / (float)cardsPerPage);
    if (totalPages < 1) totalPages = 1;
    currentPage = 0;
}

void DeckViewer::Show(vector<DrawableObject*>& objectsList)
{
    if (isActive) return;

    isActive = true;
    objectsList.push_back(background);
    createCardVisuals(objectsList);
}

void DeckViewer::Hide(vector<DrawableObject*>& objectsList)
{
    if (!isActive) return;

    isActive = false;

    auto it = find(objectsList.begin(), objectsList.end(), background);
    if (it != objectsList.end())
        objectsList.erase(it);

    clearCardVisuals(objectsList);
}

// Helper: clone an ImageObject by sharing its texture
ImageObject* DeckViewer::cloneImage(ImageObject* src, const glm::vec3& pos, float w, float h)
{
    ImageObject* copy = new ImageObject();
    copy->SetTextureId(src->GetTextureId()); // Share texture, don't reload
    copy->SetPosition(pos);
    copy->SetSize(w, h);
    return copy;
}

void DeckViewer::createCardVisuals(vector<DrawableObject*>& objectsList)
{
    clearCardVisuals(objectsList);

    int startIndex = currentPage * cardsPerPage;
    int endIndex = min(startIndex + cardsPerPage, (int)deck.size());

    // Same size as original cards
    const float W = 280.0f;
    const float H = 410.0f;
    float horizontalSpacing = 320.0f;
    float verticalSpacing = 470.0f;

    // Center the 5x2 grid in the draw area (1920x1080)
    float startX = -2.0f * horizontalSpacing;
    float topY = verticalSpacing * 0.5f;

    int cardIndex = 0;
    for (int i = startIndex; i < endIndex; i++)
    {
        Card* card = deck[i];

        int row = cardIndex / 5;
        int col = cardIndex % 5;

        float x = startX + (col * horizontalSpacing);
        float y = topY - (row * verticalSpacing);
        glm::vec3 cardPos(x, y, 0);

        // Ensure the card has visuals created so we can read textures
        if (!card->HasVisuals())
            card->CreateVisuals();

        DeckCardView view;

        // Clone each image layer from the card (sharing textures)
        if (card->GetBackground())
        {
            ImageObject* copy = cloneImage(card->GetBackground(), cardPos, W, -H);
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        if (card->GetStarOverlay())
        {
            ImageObject* copy = cloneImage(card->GetStarOverlay(), cardPos, W, -H);
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        if (card->GetTypeIcon())
        {
            ImageObject* copy = cloneImage(card->GetTypeIcon(), cardPos, W, -H);
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        if (card->GetVisual())
        {
            ImageObject* copy = cloneImage(card->GetVisual(), cardPos, W, -H);
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        if (card->GetCardFrame())
        {
            ImageObject* copy = cloneImage(card->GetCardFrame(), cardPos, W, -H);
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        // Clone text layers by sharing the texture
        if (card->GetNameText())
        {
            TextObject* src = card->GetNameText();
            TextObject* copy = new TextObject();
            copy->SetTextureId(src->GetTextureId());
            float nameW = src->GetSize().x-2.0f;
            float nameH = src->GetSize().y-2.5f;
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
            float leftAnchorX = (cardPos.x - (W * 0.5f)) + (local.x * W);
            float centeredX = leftAnchorX + (copy->GetSize().x * 0.5f) +55.0f;
            copy->SetPosition(glm::vec3(centeredX, cardPos.y + (local.y * H), 0));
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        if (card->GetDescriptionText())
        {
            TextObject* src = card->GetDescriptionText();
            TextObject* copy = new TextObject();
            copy->SetTextureId(src->GetTextureId());
            copy->SetSize(src->GetSize().x, src->GetSize().y);
            // Left-anchor X using local offset as margin from card's left edge
            glm::vec3 local = src->GetLocalPosition();
            float leftAnchorX = (cardPos.x - (W * 0.5f)) + (local.x * W);
            float centeredX = leftAnchorX + (copy->GetSize().x * 0.5f);
            copy->SetPosition(glm::vec3(centeredX, cardPos.y + (local.y * H), 0));
            view.layers.push_back(copy);
            objectsList.push_back(copy);
        }

        cardViews.push_back(view);
        cardIndex++;
    }
}

void DeckViewer::clearCardVisuals(vector<DrawableObject*>& objectsList)
{
    for (auto& view : cardViews)
    {
        for (auto* layer : view.layers)
        {
            auto it = find(objectsList.begin(), objectsList.end(), layer);
            if (it != objectsList.end())
                objectsList.erase(it);
            delete layer;
        }
    }
    cardViews.clear();
}

void DeckViewer::NextPage(vector<DrawableObject*>& objectsList)
{
    if (!isActive) return;

    currentPage++;
    if (currentPage >= totalPages)
        currentPage = totalPages - 1;

    createCardVisuals(objectsList);
}

void DeckViewer::PrevPage(vector<DrawableObject*>& objectsList)
{
    if (!isActive) return;

    currentPage--;
    if (currentPage < 0)
        currentPage = 0;

    createCardVisuals(objectsList);
}
