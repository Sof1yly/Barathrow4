#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>


#include "Card.h"
#include "DrawableObject.h"
#include "ImageObject.h"

struct CardView
{
    Card* cardData = nullptr;  // which logical card
    ImageObject* background = nullptr;
    ImageObject* cardFrame = nullptr;    // card_fr_XX.png 
    ImageObject* visualFrame = nullptr;  // card_vf_XX.png 
    ImageObject* visual = nullptr;       
    ImageObject* typeIcon = nullptr;    // card_ty_XX.png
    ImageObject* starBase = nullptr;    // star_gy3.png
    ImageObject* starOverlay = nullptr; // star_gd1/2/3.png
};

class Hand
{
private:
    std::vector<CardView> views;
    ImageObject* selectedView = nullptr;
    ImageObject* hoveredView = nullptr;

    std::unordered_map<ImageObject*, glm::vec3> origPos;
    std::unordered_map<ImageObject*, glm::vec2> origSize;
    std::unordered_map<ImageObject*, float> origRot;
    

    std::unordered_map<ImageObject*, size_t> origIndices;

    bool hitTestBase(ImageObject* v, const glm::vec3& p) const;    // use orig fan pos
    bool hitTestCurrent(ImageObject* v, const glm::vec3& p) const; // use current pos
    void layoutViews();
    void liftForHover(ImageObject* v, std::vector<DrawableObject*>& objectsList);
    void clearHover(std::vector<DrawableObject*>& objectsList);

    std::vector<ImageObject*> getAllImagesFromView(const CardView& cv) const;

public:
    ImageObject* GetSelectedView() const { return selectedView; }


    void CreateVisualHand(int cardCount,std::vector<DrawableObject*>& objectsList,const std::vector<Card*>& cardData);


    int GetCardCount() const;


    void AddCards(const std::vector<Card*>& cardsToAdd,std::vector<DrawableObject*>& objectsList);

    void Clear(std::vector<DrawableObject*>& objectsList);


    std::vector<Card*> CollectAllCardData() const;


    ImageObject* PeekAt(const glm::vec3& mouseWorld);


    void UpdateHover(const glm::vec3& mouseWorld, bool isDragging, std::vector<DrawableObject*>& objectsList);

 
    void RemoveView(ImageObject* view, std::vector<DrawableObject*>& objectsList);

    Card* FindCardByImage(ImageObject* img);

    bool TrySelectAt(const glm::vec3& mouseWorld);
    void Deselect();
};
