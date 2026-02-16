#include "CardSystem.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

using namespace std;

// Bezier helper
static inline glm::vec3 QuadraticBezier(const glm::vec3& P0,const glm::vec3& C,const glm::vec3& P1,float t)
{
    float u = 1.0f - t;
    return (u * u) * P0 + 2.0f * u * t * C + (t * t) * P1;
}


CardSystem::CardSystem() {}

CardSystem::~CardSystem() {}



bool CardSystem::LoadData(const string& patternFile,const string& cardFile,string* outError)
{
    if (!dataLoader.loadPatternsFromFile(patternFile, outError))
        return false;
    if (!dataLoader.loadFromFile(cardFile, outError))
        return false;

    deck = dataLoader.getCards();
    return true;
}

// ============================================================
// UI Initialization
// ============================================================

void CardSystem::InitUI(vector<DrawableObject*>& objectsList)
{
    // Discard pile background card 
    discardPileBG = new ImageObject();
    discardPileBG->SetSize(245.0f, -360.0f);
    discardPileBGPos = glm::vec3(-795.0f, -325.0f, 9.0f);
    discardPileBG->SetPosition(discardPileBGPos);
    discardPileBG->SetTexture("../Resource/Texture/cards/BG_back/Gy_Back_Card.PNG");
    objectsList.push_back(discardPileBG);

    // Discard pile button (LEFT)
    discardPileButton = new ImageObject();
    discardPileButton->SetSize(260.0f, -275.0f);
    discardPileButton->SetPosition(glm::vec3(-800.0f, -385.0f, 10.0f));
    discardPileButton->SetTexture("../Resource/Texture/cards/DiscardPile.png");
    objectsList.push_back(discardPileButton);

    // Draw pile background card
    drawPileBG = new ImageObject();
    drawPileBG->SetSize(245.0f, -360.0f);
    drawPileBGPos = glm::vec3(805.0f, -325.0f, 9.0f);
    drawPileBG->SetPosition(drawPileBGPos);
    drawPileBG->SetTexture("../Resource/Texture/cards/BG_back/Gy_Back_Card.PNG");
    objectsList.push_back(drawPileBG);

    // Draw pile button (RIGHT)
    drawPileButton = new ImageObject();
    drawPileButton->SetSize(260.0f, -275.0f);
    drawPileButton->SetPosition(glm::vec3(800.0f, -385.0f, 10.0f));
    drawPileButton->SetTexture("../Resource/Texture/cards/DrawPile.png");
    objectsList.push_back(drawPileButton);

    // Create drop zones
    CreateDropZones(objectsList);

    // Set initial visibility
    UpdatePileVisuals();
}

// ============================================================
// Deck Management
// ============================================================

void CardSystem::ShuffleDeck()
{
    if (deck.empty()) return;

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
}

void CardSystem::DealNewHand(int cardCount,std::vector<DrawableObject*>& objectsList)
{
    if (cardCount <= 0) return;

    hand.Clear(objectsList);

    int drawCount = std::min(cardCount, (int)deck.size());
    std::vector<Card*> drawn;
    for (int i = 0; i < drawCount && !deck.empty(); ++i)
    {
        drawn.push_back(deck.back());
        deck.pop_back();
    }

    if (!drawn.empty())
    {
        hand.AddCards(drawn, objectsList);

        for (Card* c : drawn) {
            if (c != nullptr) {
                std::cout << c->getName() << std::endl;
            }
        }
    }

    UpdatePileVisuals();
}

void CardSystem::DiscardHandAndDraw(int cardCount,std::vector<DrawableObject*>& objectsList)
{
    cout << "============================" << endl;
    cout << "[UI] Draw from Deck (Discard current hand)" << endl;

    // 1) Collect all card data from current hand
    std::vector<Card*> cardsInHand = hand.CollectAllCardData();

    // 2) Move all cards from hand to discard pile
    if (!cardsInHand.empty())
    {
        discard.insert(discard.end(), cardsInHand.begin(), cardsInHand.end());
    }

    // 3) Clear the visual hand
    hand.Clear(objectsList);

    // 4) If deck is empty, shuffle discard pile back into deck
    if (deck.empty() && !discard.empty())
    {
        cout << "  Deck is empty. Moving " << discard.size()<< " cards from discard to deck" << endl;
        deck.insert(deck.end(), discard.begin(), discard.end());
        discard.clear();
        ShuffleDeck();
    }

    // 5) Draw new hand
    int drawCount = std::min(cardCount, (int)deck.size());

    if (drawCount > 0)
    {
        std::vector<Card*> drawn;
        for (int i = 0; i < drawCount; ++i)
        {
            drawn.push_back(deck.back());
            deck.pop_back();
        }

        hand.AddCards(drawn, objectsList);

        cout << "  Drew " << drawCount << " new cards" << endl;
        for (Card* c : drawn) {
            if (c != nullptr) {
                std::cout << "    - " << c->getName() << std::endl;
            }
        }
    }
    else
    {
        cout << "  No cards available to draw!" << endl;
    }

    UpdatePileVisuals();
}

void CardSystem::DiscardCard(Card* card)
{
    if (card) {
        discard.push_back(card);
        UpdatePileVisuals();
    }
}

// ============================================================
// Drop Zones
// ============================================================

void CardSystem::CreateDropZones(std::vector<DrawableObject*>& list)
{
    if (dropZonesCreated) return;
    dropZonesCreated = true;

    const float Z = 1.0f;
    const float SIDE_W = 390.0f;
    const float SIDE_H = 530.0f;
    const float MID_W = 710.0f;
    const float MID_H = 290.0f;
    const float BOARD_CENTER_Y = 200.0f;
    const float SIDE_X_OFFSET = 600.0f;
    const float SIDE_Y = BOARD_CENTER_Y;
    const float UPPER_Y = BOARD_CENTER_Y + 180.0f;
    const float BOTTOM_Y = BOARD_CENTER_Y - 180.0f;

    // LEFT
    dropZones[0] = new GameObject();
    dropZones[0]->SetSize(SIDE_W, SIDE_H);
    dropZones[0]->SetPosition(glm::vec3(-SIDE_X_OFFSET, SIDE_Y, Z));
    dropZones[0]->SetColor(1.0f, 0.6f, 0.8f, 0.0f);

    // TOP
    dropZones[1] = new GameObject();
    dropZones[1]->SetSize(MID_W, MID_H);
    dropZones[1]->SetPosition(glm::vec3(0.0f, UPPER_Y, Z));
    dropZones[1]->SetColor(1.0f, 0.6f, 0.8f, 0.0f);

    // BOTTOM
    dropZones[2] = new GameObject();
    dropZones[2]->SetSize(MID_W, MID_H);
    dropZones[2]->SetPosition(glm::vec3(0.0f, BOTTOM_Y, Z));
    dropZones[2]->SetColor(1.0f, 0.6f, 0.8f, 0.0f);

    // RIGHT
    dropZones[3] = new GameObject();
    dropZones[3]->SetSize(SIDE_W, SIDE_H);
    dropZones[3]->SetPosition(glm::vec3(SIDE_X_OFFSET, SIDE_Y, Z));
    dropZones[3]->SetColor(1.0f, 0.6f, 0.8f, 0.0f);

    for (int i = 0; i < 4; ++i) {
        list.push_back(dropZones[i]);
        dropZoneSavedPos[i] = dropZones[i]->GetPosition();
        dropZones[i]->SetPosition(glm::vec3(dropZoneSavedPos[i].x, -10000.0f, dropZoneSavedPos[i].z));
    }

    dropZonesVisible = false;
}

void CardSystem::ShowDropZones()
{
    if (!dropZonesCreated || dropZonesVisible) return;
    for (int i = 0; i < 4; i++) {
        dropZones[i]->SetPosition(dropZoneSavedPos[i]);
    }
        
    dropZonesVisible = true;
}

void CardSystem::HideDropZones()
{
    if (!dropZonesCreated || !dropZonesVisible) return;
    for (int i = 0; i < 4; i++) {
        auto p = dropZoneSavedPos[i];
        dropZones[i]->SetPosition(glm::vec3(p.x, -10000.0f, p.z));
    }
    dropZonesVisible = false;
}

int CardSystem::HitDropZone(const glm::vec3& p) const
{
    for (int i = 0; i < 4; i++) {
        if (IsPointInsideZone(p, dropZones[i])) {
            return i;
        }
    }

            
    return -1;
}

// ============================================================
// Dragging
// ============================================================

void CardSystem::BeginDrag(ImageObject* card, const glm::vec3& mouseWorld,std::vector<DrawableObject*>& objectsList)
{
    if (isDragging || !card) return;

    EnsureBezierSegments(objectsList);
    ShowDropZones();

    isDragging = true;
    draggingCard = card;

    dragStartPos = card->GetPosition();
    dragMouseWorld = mouseWorld;
    dragAnchor = dragStartPos;

    std::vector<ImageObject*> allLayers = hand.GetAllLayersForCard(card);
    for (ImageObject* layer : allLayers) {
        if (layer) {
            layer->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 600.0f));
        }
    }

    UpdateBezier(draggingCard->GetPosition(), mouseWorld);
}

void CardSystem::UpdateDrag(const glm::vec3& mouseWorld)
{
    if (!isDragging || !draggingCard) return;

    dragMouseWorld = mouseWorld;

    std::vector<ImageObject*> allLayers = hand.GetAllLayersForCard(draggingCard);
    for (ImageObject* layer : allLayers) {
        if (layer) {
            layer->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 600.0f));
        }
    }

    glm::vec3 anchor = draggingCard->GetPosition();
    UpdateBezier(anchor, mouseWorld);
}

void CardSystem::EndDragCancel(const glm::vec3& mouseWorld,std::vector<DrawableObject*>& objectsList)
{
    if (!isDragging || !draggingCard) return;

    std::vector<ImageObject*> allLayers = hand.GetAllLayersForCard(draggingCard);
    for (ImageObject* layer : allLayers) {
        if (layer) {
            layer->SetPosition(glm::vec3(dragStartPos.x, dragStartPos.y, 300.0f));
        }
    }

    HideBezier();
    HideDropZones();
    hand.UpdateHover(mouseWorld, false, objectsList);

    isDragging = false;
    draggingCard = nullptr;
    pendingCard = nullptr;
}

void CardSystem::EndDragConfirm(ImageObject* card, std::vector<DrawableObject*>& objectsList)
{
    if (!card) return;

    Card* cardData = hand.FindCardByImage(card);

    hand.RemoveView(card, objectsList);

    if (cardData) {
        discard.push_back(cardData);
    }

    HideBezier();
    HideDropZones();

    isDragging = false;
    draggingCard = nullptr;
    pendingCard = nullptr;

    UpdatePileVisuals();
}

// ============================================================
// Hand Queries / Hover
// ============================================================

void CardSystem::UpdateHover(const glm::vec3& mousePos, bool dragging,std::vector<DrawableObject*>& objectsList)
{
    hand.UpdateHover(mousePos, dragging, objectsList);
}

ImageObject* CardSystem::PeekAt(const glm::vec3& mousePos)
{
    return hand.PeekAt(mousePos);
}

Card* CardSystem::FindCardByImage(ImageObject* img)
{
    return hand.FindCardByImage(img);
}

std::vector<ImageObject*> CardSystem::GetAllLayersForCard(ImageObject* card)
{
    return hand.GetAllLayersForCard(card);
}

void CardSystem::RemoveCardView(ImageObject* card,std::vector<DrawableObject*>& objectsList)
{
    hand.RemoveView(card, objectsList);
}


bool CardSystem::IsDrawPileClicked(const glm::vec3& mousePos) const
{
    return drawPileButton && IsPointInsideZone(mousePos, drawPileButton);
}

bool CardSystem::IsDiscardPileClicked(const glm::vec3& mousePos) const
{
    return discardPileButton && IsPointInsideZone(mousePos, discardPileButton);
}



void CardSystem::PrintDiscardPile() const
{
    cout << "==== DISCARD PILE CONTENTS ====" << endl;
    cout << "Total cards in discard pile: " << discard.size() << endl;

    if (!discard.empty())
    {
        for (size_t i = 0; i < discard.size(); i++)
        {
            Card* c = discard[i];
            if (c != nullptr) {
                std::cout << "  [" << (i + 1) << "] " << c->getName() << std::endl;
            }
        }
    }
    else
    {
        cout << "  Discard pile is empty!" << endl;
    }
    cout << "=============================" << endl;
}

void CardSystem::PrintDrawDeck() const
{
    cout << "[UI] View Draw Deck" << endl;
    cout << "=== DRAW DECK CONTENTS ===" << endl;
    cout << "Total cards in draw deck: " << deck.size() << endl;

    if (!deck.empty())
    {
        for (size_t i = 0; i < deck.size(); i++)
        {
            Card* c = deck[i];
            if (c != nullptr) {
                std::cout << "  [" << (i + 1) << "] " << c->getName() << std::endl;
            }
        }
    }
    else
    {
        cout << "  Draw deck is empty!" << endl;
    }
    cout << "=============================" << endl;
}



void CardSystem::Clear(std::vector<DrawableObject*>& objectsList)
{
    hand.Clear(objectsList);
    bezierSegments.clear();
    deck.clear();
    discard.clear();
}

void CardSystem::Reset(std::vector<DrawableObject*>& objectsList)
{
    hand.Clear(objectsList);

    for (int i = 0; i < 4; ++i)
        dropZones[i] = nullptr;

    bezierSegments.clear();

    dropZonesCreated = false;
    dropZonesVisible = false;
    bezierCreated = false;
    isDragging = false;
    isHolding = false;
    draggingCard = nullptr;
    pendingCard = nullptr;
    drawPileButton = nullptr;
    discardPileButton = nullptr;
    drawPileBG = nullptr;
    discardPileBG = nullptr;

    deck = dataLoader.getCards();
    discard.clear();
    ShuffleDeck();
}



void CardSystem::EnsureBezierSegments(std::vector<DrawableObject*>& list)
{
    if (bezierCreated) return;
    bezierCreated = true;

    bezierSegments.reserve(BEZIER_SEGMENTS);
    for (int i = 0; i < BEZIER_SEGMENTS; ++i) {
        auto* seg = new GameObject();
        seg->SetColor(1.0f, 0.6f, 0.85f);
        seg->SetSize(1.0f, 6.0f);
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 0.0f));
        bezierSegments.push_back(seg);
        list.push_back(seg);
    }
}

void CardSystem::UpdateBezier(const glm::vec3& P0, const glm::vec3& P1)
{
    if (!bezierCreated) return;

    float midY = 0.5f * (P0.y + P1.y);
    float dir = (midY < screenCenterY) ? 1.0f : -1.0f;

    float liftAmount = 220.0f;
    glm::vec3 mid = 0.5f * (P0 + P1);
    glm::vec3 C = mid + glm::vec3(0.0f, dir * liftAmount, 0.0f);

    for (int i = 0; i < BEZIER_SEGMENTS; ++i) {
        float t0 = (float)i / (float)BEZIER_SEGMENTS;
        float t1 = (float)(i + 1) / (float)BEZIER_SEGMENTS;

        glm::vec3 A = QuadraticBezier(P0, C, P1, t0);
        glm::vec3 B = QuadraticBezier(P0, C, P1, t1);
        glm::vec3 D = B - A;

        float len = glm::length(D);
        auto* seg = bezierSegments[i];

        if (len < 0.001f) {
            seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 500.0f));
            continue;
        }

        glm::vec3 midAB = 0.5f * (A + B);
        float angleRad = std::atan2(D.y, D.x);
        float angleDeg = angleRad * 180.0f / 3.14159265f;

        seg->SetPosition(glm::vec3(midAB.x, midAB.y, 0.0f));
        seg->SetSize(len, 6.0f);
        seg->SetRotate(angleDeg);
    }
}

void CardSystem::HideBezier()
{
    for (auto* seg : bezierSegments){
        seg->SetPosition(glm::vec3(99999.0f, 99999.0f, 0.0f));
    }
       
}

void CardSystem::UpdatePileVisuals()
{
    const float HIDDEN_Y = -99999.0f;

    // Discard pile BG: show only when discard has cards
    if (discardPileBG)
    {
        if (!discard.empty())
            discardPileBG->SetPosition(discardPileBGPos);
        else
            discardPileBG->SetPosition(glm::vec3(discardPileBGPos.x, HIDDEN_Y, discardPileBGPos.z));
    }

    // Draw pile BG: show only when deck has cards
    if (drawPileBG)
    {
        if (!deck.empty())
            drawPileBG->SetPosition(drawPileBGPos);
        else
            drawPileBG->SetPosition(glm::vec3(drawPileBGPos.x, HIDDEN_Y, drawPileBGPos.z));
    }
}

bool CardSystem::IsPointInsideZone(const glm::vec3& p, DrawableObject* zone) const
{
    if (!zone) return false;

    glm::vec3 zpos = zone->GetPosition();
    glm::vec2 zsize = zone->GetSize();

    float halfW = std::abs(zsize.x) * 0.5f;
    float halfH = std::abs(zsize.y) * 0.5f;

    return (p.x >= zpos.x - halfW && p.x <= zpos.x + halfW && p.y >= zpos.y - halfH && p.y <= zpos.y + halfH);
}
