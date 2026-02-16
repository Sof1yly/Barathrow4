#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>


#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"

struct CardView
{
    Card* cardData = nullptr;
    ImageObject* background = nullptr;
    ImageObject* cardFrame = nullptr;
    ImageObject* visual = nullptr;
    ImageObject* typeIcon = nullptr;
    ImageObject* starOverlay = nullptr;
};

class Hand
{
private:
    vector<CardView> views;
    ImageObject* selectedView = nullptr;
    ImageObject* hoveredView = nullptr;
    ImageObject* draggingView = nullptr;

    unordered_map<ImageObject*, glm::vec3> origPos;
    unordered_map<ImageObject*, glm::vec2> origSize;
    unordered_map<ImageObject*, float> origRot;
    

    unordered_map<ImageObject*, size_t> origIndices;

    bool hitTestBase(ImageObject* v, const glm::vec3& p) const;    // use orig fan pos
    bool hitTestCurrent(ImageObject* v, const glm::vec3& p) const; // use current pos
    void layoutViews();
    void liftForHover(ImageObject* v, vector<DrawableObject*>& objectsList);
    void clearHover(vector<DrawableObject*>& objectsList);

    vector<DrawableObject*> getAllImagesFromView(const CardView& cv) const;

public:
    ImageObject* GetSelectedView() const { return selectedView; }
    void SetDragging(ImageObject* card) { draggingView = card; }
    void ClearDragging() { draggingView = nullptr; }

    void RestoreLayout(ImageObject* card, vector<DrawableObject*>& objectsList);

    void CreateVisualHand(int cardCount,vector<DrawableObject*>& objectsList,const vector<Card*>& cardData);


    int GetCardCount() const;


    void AddCards(const vector<Card*>& cardsToAdd,vector<DrawableObject*>& objectsList);

    void Clear(std::vector<DrawableObject*>& objectsList);


    vector<Card*> CollectAllCardData() const;


    ImageObject* PeekAt(const glm::vec3& mouseWorld);


    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging, vector<DrawableObject*>& objectsList);

 
    void RemoveView(ImageObject* view, vector<DrawableObject*>& objectsList);

    Card* FindCardByImage(ImageObject* img);

    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();

    
    vector<DrawableObject*> GetAllLayersForCard(ImageObject* anyLayer);
};
