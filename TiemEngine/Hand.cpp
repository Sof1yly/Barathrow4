#include "Hand.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include <iostream>
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

// Helper function to get card frame filename based on rarity
static std::string getCardFrameName(int rarity)
{
    switch (rarity) {
    case 1: return "BG_Frame/card_fr_gy.png";
    case 2: return "BG_Frame/card_fr_gn.png";
    case 3: return "BG_Frame/card_fr_br.png";
    case 4: return "BG_Frame/card_fr_or.png";
    case 5: return "BG_Frame/card_fr_pk.png";
    default: return "BG_Frame/card_fr_gy.png";
    }
}

// Helper function to get visual frame filename based on rarity
static std::string getVisualFrameName(int rarity)
{
    switch (rarity) {
    case 1: return "BG_viFrame/card_vf_gy.png";
    case 2: return "BG_viFrame/card_vf_gn.png";
    case 3: return "BG_viFrame/card_vf_bl.png";
    case 4: return "BG_viFrame/card_vf_or.png";
    case 5: return "BG_viFrame/card_vf_pk.png";
    default: return "BG_viFrame/card_vf_gy.png";
    }
}

// Helper function to get type icon filename based on type
static std::string getTypeIconName(int type)
{
    switch (type) {
    case 1: return "BG_Type/card_ty_atk.png";
    case 2: return "BG_Type/card_ty_mv.png";
    case 3: return "BG_Type/card_ty_er.png";
    case 4: return "BG_Type/card_ty_bf.png";
    case 5: return "BG_Type/card_ty_db.png";
    default: return "BG_Type/card_ty_atk.png";
    }
}

// Helper function to get star base filename (always gray 3 stars)
static std::string getStarBaseName()
{
    return "BG_Stars/star_gy3.png";
}

// Helper function to get star overlay filename based on level
static std::string getStarOverlayName(int level)
{
    switch (level) {
    case 1: return "BG_Stars/star_gd1.png";
    case 2: return "BG_Stars/star_gd2.png";
    case 3: return "BG_Stars/star_gd3.png";
    default: return ""; // no overlay for level 0
    }
}

// Helper function to get background filename
static std::string getBackgroundName()
{
    return "BG_card/card_bg.png";
}

std::vector<ImageObject*> Hand::getAllImagesFromView(const CardView& cv) const
{
    std::vector<ImageObject*> images;
    if (cv.background) images.push_back(cv.background);
    if (cv.starBase) images.push_back(cv.starBase);
    if (cv.starOverlay) images.push_back(cv.starOverlay);
    if (cv.typeIcon) images.push_back(cv.typeIcon);
    if (cv.visual) images.push_back(cv.visual);
    if (cv.cardFrame) images.push_back(cv.cardFrame);
    if (cv.visualFrame) images.push_back(cv.visualFrame);
    return images;
}

bool Hand::hitTestBase(ImageObject* v, const glm::vec3& p) const
{
    if (!v) return false;

    // Use original fan transform if we have it
    glm::vec3 pos = v->GetPosition();
    glm::vec2 sz = v->GetSize();

    auto itP = origPos.find(v);
    if (itP != origPos.end())
        pos = itP->second;

    auto itS = origSize.find(v);
    if (itS != origSize.end())
        sz = itS->second;

    float halfW = std::fabs(sz.x) * 0.5f;
    float halfH = std::fabs(sz.y) * 0.5f;

    return (p.x >= pos.x - halfW && p.x <= pos.x + halfW &&
        p.y >= pos.y - halfH && p.y <= pos.y + halfH);
}

bool Hand::hitTestCurrent(ImageObject* v, const glm::vec3& p) const
{
    if (!v) return false;

    glm::vec3 pos = v->GetPosition();
    glm::vec2 sz = v->GetSize();

    float halfW = std::fabs(sz.x) * 0.5f;
    float halfH = std::fabs(sz.y) * 0.5f;

    return (p.x >= pos.x - halfW && p.x <= pos.x + halfW &&
        p.y >= pos.y - halfH && p.y <= pos.y + halfH);
}




void Hand::layoutViews()
{
    int count = (int)views.size();
    if (count <= 0) return;

    const float W = 280.0f;
    const float H = 410.0f;
    const float handY = -610.0f;    // baseline for the hand at bottom
    const float baseRadius = 2100.0f;    // bigger = flatter
    const float R = baseRadius + 120.0f;
    const float Cx = 0.0f;
    const float Cy = handY - R;
    const float sepFactor = 0.65f;
    const float sink = 4.0f;

    float arcSpacing = W * sepFactor;
    float stepDeg = (arcSpacing / R) * RAD2DEG;
    float spanDeg = (count > 1) ? stepDeg * (count - 1) : 0.0f;
    float startDeg = -0.5f * spanDeg;

    origPos.clear();
    origSize.clear();
    origRot.clear();

    for (int i = 0; i < count; ++i)
    {
        const CardView& cv = views[i];
        std::vector<ImageObject*> allImages = getAllImagesFromView(cv);

        if (allImages.empty()) continue;

        float deg = startDeg + stepDeg * i;
        float ang = deg * (PI / 180.0f);

        float rimX = Cx + std::sinf(ang) * R;
        float rimY = Cy + std::cosf(ang) * R;

        float nx = std::sinf(ang);
        float ny = std::cosf(ang);
        float tx = ny;
        float ty = -nx;

        float px = rimX + nx * (H * 0.5f - sink);
        float py = rimY + ny * (H * 0.5f - sink);

        float rotDeg = std::atan2f(ty, tx) * RAD2DEG;

        // Apply same transformation to all layers of this card
        for (ImageObject* img : allImages) {
            if (!img) continue;

            img->SetPosition(glm::vec3(px, py, 0.0f));
            img->SetSize(W, -H);
            img->SetRotate(rotDeg);

            origPos[img] = img->GetPosition();
            origSize[img] = img->GetSize();
            origRot[img] = img->GetRotate();
        }
    }
}

void Hand::liftForHover(ImageObject* v)
{
    if (!v) return;

    // Find which CardView this image belongs to
    CardView* targetView = nullptr;
    for (auto& cv : views) {
        std::vector<ImageObject*> images = getAllImagesFromView(cv);
        for (ImageObject* img : images) {
            if (img == v) {
                targetView = &cv;
                break;
            }
        }
        if (targetView) break;
    }

    if (!targetView) return;

    // Get all images from this card
    std::vector<ImageObject*> allImages = getAllImagesFromView(*targetView);

    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 135.0f;   // how high above fan
    const float HOVER_Z = 420.0f;   // above other cards
    const float HOVER_SCALE = 1.35f;     // change this to adjust hover size

    glm::vec3 newPos(basePos.x, basePos.y + HOVER_OFFSET_Y, HOVER_Z);
    glm::vec2 newSize(baseSize.x * HOVER_SCALE, baseSize.y * HOVER_SCALE);

    // Apply hover effect to all layers
    for (ImageObject* img : allImages) {
        if (!img) continue;
        img->SetPosition(newPos);
        img->SetSize(newSize.x, newSize.y);
        img->SetRotate(0.0f); // straight up
    }
}

void Hand::clearHover()
{
    if (!hoveredView) return;

    // Find which CardView this image belongs to
    CardView* targetView = nullptr;
    for (auto& cv : views) {
        std::vector<ImageObject*> images = getAllImagesFromView(cv);
        for (ImageObject* img : images) {
            if (img == hoveredView) {
                targetView = &cv;
                break;
            }
        }
        if (targetView) break;
    }

    if (!targetView) {
        hoveredView = nullptr;
        return;
    }

    // Get all images and restore original positions
    std::vector<ImageObject*> allImages = getAllImagesFromView(*targetView);
    for (ImageObject* img : allImages) {
        if (!img) continue;
        if (origPos.count(img))  img->SetPosition(origPos[img]);
        if (origSize.count(img)) img->SetSize(origSize[img].x, origSize[img].y);
        if (origRot.count(img))  img->SetRotate(origRot[img]);
    }

    hoveredView = nullptr;
}

void Hand::CreateVisualHand(int cardCount,
    std::vector<DrawableObject*>& objectsList,
    const std::vector<Card*>& cardData)
{
    views.clear();
    hoveredView = nullptr;
    selectedView = nullptr;
    origPos.clear();
    origSize.clear();
    origRot.clear();

    int count = std::min(cardCount, (int)cardData.size());

    for (int i = 0; i < count; ++i)
    {
        Card* data = cardData[i];

        // Get card visual properties
        int level = data->getLevel();    // 0-3 stars
        int rarity = data->getRarity();  // 1-5 rarity
        int type = data->getType();      // 1-5 type

        CardView cv;
        cv.cardData = data;

        std::string basePath = "../Resource/Texture/cards/";

        // RENDER ORDER (bottom to top):

        // 1. Background (bottom layer)
        cv.background = new ImageObject();
        cv.background->SetSize(280.0f, -410.0f);
        cv.background->SetTexture(basePath + getBackgroundName());
        objectsList.push_back(cv.background);

        // 2. Star Base (gray stars - always rendered)
        cv.starBase = new ImageObject();
        cv.starBase->SetSize(280.0f, -410.0f);
        cv.starBase->SetTexture(basePath + getStarBaseName());
        objectsList.push_back(cv.starBase);

        // 3. Star Overlay (golden stars based on level) - only if level > 0
        if (level > 0) {
            cv.starOverlay = new ImageObject();
            cv.starOverlay->SetSize(280.0f, -410.0f);
            cv.starOverlay->SetTexture(basePath + getStarOverlayName(level));
            objectsList.push_back(cv.starOverlay);
        }

        // 4. Type Icon
        cv.typeIcon = new ImageObject();
        cv.typeIcon->SetSize(280.0f, -410.0f);
        cv.typeIcon->SetTexture(basePath + getTypeIconName(type));
        objectsList.push_back(cv.typeIcon);

        // 5. Main Visual (center card image)
        cv.visual = new ImageObject();
        cv.visual->SetSize(280.0f, -410.0f);
        cv.visual->SetTexture(basePath + "Sweep_attack.png"); // Replace with actual visual
        objectsList.push_back(cv.visual);

        // 6. Card Frame (based on rarity)
        cv.cardFrame = new ImageObject();
        cv.cardFrame->SetSize(280.0f, -410.0f);
        cv.cardFrame->SetTexture(basePath + getCardFrameName(rarity));
        objectsList.push_back(cv.cardFrame);

        // 7. Visual Frame (top layer - based on rarity)
        cv.visualFrame = new ImageObject();
        cv.visualFrame->SetSize(280.0f, -410.0f);
        cv.visualFrame->SetTexture(basePath + getVisualFrameName(rarity));
        objectsList.push_back(cv.visualFrame);

        views.push_back(cv);
    }

    layoutViews();
}

int Hand::GetCardCount() const
{
    return static_cast<int>(views.size());
}

void Hand::AddCards(const std::vector<Card*>& cardsToAdd,
    std::vector<DrawableObject*>& objectsList)
{
    for (Card* data : cardsToAdd)
    {
        if (!data) {
            continue;
        }

        // Get card visual properties
        int level = data->getLevel();
        int rarity = data->getRarity();
        int type = data->getType();

        CardView cv;
        cv.cardData = data;

        std::string basePath = "../Resource/Texture/cards/";

        // RENDER ORDER (bottom to top):

        // 1. Background (bottom layer)
        cv.background = new ImageObject();
        cv.background->SetSize(280.0f, -410.0f);
        cv.background->SetTexture(basePath + getBackgroundName());
        objectsList.push_back(cv.background);

        // 2. Star Base (gray stars - always rendered)
        cv.starBase = new ImageObject();
        cv.starBase->SetSize(280.0f, -410.0f);
        cv.starBase->SetTexture(basePath + getStarBaseName());
        objectsList.push_back(cv.starBase);

        // 3. Star Overlay (golden stars based on level) - only if level > 0
        if (level > 0) {
            cv.starOverlay = new ImageObject();
            cv.starOverlay->SetSize(280.0f, -410.0f);
            cv.starOverlay->SetTexture(basePath + getStarOverlayName(level));
            objectsList.push_back(cv.starOverlay);
        }

        // 4. Type Icon
        cv.typeIcon = new ImageObject();
        cv.typeIcon->SetSize(280.0f, -410.0f);
        cv.typeIcon->SetTexture(basePath + getTypeIconName(type));
        objectsList.push_back(cv.typeIcon);

        // 5. Main Visual
        cv.visual = new ImageObject();
        cv.visual->SetSize(280.0f, -410.0f);
        cv.visual->SetTexture(basePath + "Sweep_attack.png");
        objectsList.push_back(cv.visual);

        // 6. Card Frame
        cv.cardFrame = new ImageObject();
        cv.cardFrame->SetSize(280.0f, -410.0f);
        cv.cardFrame->SetTexture(basePath + getCardFrameName(rarity));
        objectsList.push_back(cv.cardFrame);

        // 7. Visual Frame (top layer)
        cv.visualFrame = new ImageObject();
        cv.visualFrame->SetSize(280.0f, -410.0f);
        cv.visualFrame->SetTexture(basePath + getVisualFrameName(rarity));
        objectsList.push_back(cv.visualFrame);

        views.push_back(cv);
    }

    // re-fan whole hand
    layoutViews();
}

void Hand::Clear(std::vector<DrawableObject*>& objectsList)
{
    for (auto& cv : views)
    {
        std::vector<ImageObject*> allImages = getAllImagesFromView(cv);

        for (ImageObject* img : allImages) {
            if (!img) continue;

            auto it = std::find(objectsList.begin(), objectsList.end(), img);
            if (it != objectsList.end()) {
                objectsList.erase(it);
            }

            delete img;
        }
    }

    views.clear();
    hoveredView = nullptr;
    selectedView = nullptr;
    origPos.clear();
    origSize.clear();
    origRot.clear();
}

std::vector<Card*> Hand::CollectAllCardData() const
{
    std::vector<Card*> out;
    out.reserve(views.size());
    for (const auto& v : views)
    {
        if (v.cardData) {
            out.push_back(v.cardData);
        }
    }
    return out;
}

ImageObject* Hand::PeekAt(const glm::vec3& mouseWorld)
{
    // Check cards in reverse order (front to back)
    for (int i = (int)views.size() - 1; i >= 0; --i) {
        const CardView& cv = views[i];

        // Test against background (the main hit area)
        if (cv.background && hitTestBase(cv.background, mouseWorld)) {
            return cv.background;
        }
    }
    return nullptr;
}

void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging)
{
    if (isDragging) {
        if (hoveredView) {
            clearHover();
        }
        return;
    }

    ImageObject* hit = PeekAt(mouseWorld);

    if (hit == hoveredView) {
        return;
    }

    if (hoveredView) {
        clearHover();
    }

    if (hit) {
        hoveredView = hit;
        liftForHover(hit);
    }
}

void Hand::RemoveView(ImageObject* view)
{
    if (!view) return;

    auto it = std::find_if(views.begin(), views.end(),
        [view](const CardView& cv) {
            std::vector<ImageObject*> images;
            if (cv.background) images.push_back(cv.background);
            if (cv.starBase) images.push_back(cv.starBase);
            if (cv.starOverlay) images.push_back(cv.starOverlay);
            if (cv.typeIcon) images.push_back(cv.typeIcon);
            if (cv.visual) images.push_back(cv.visual);
            if (cv.cardFrame) images.push_back(cv.cardFrame);
            if (cv.visualFrame) images.push_back(cv.visualFrame);

            return std::find(images.begin(), images.end(), view) != images.end();
        });

    if (it != views.end()) {
        // Get all images from this view
        std::vector<ImageObject*> allImages;
        if (it->background) allImages.push_back(it->background);
        if (it->starBase) allImages.push_back(it->starBase);
        if (it->starOverlay) allImages.push_back(it->starOverlay);
        if (it->typeIcon) allImages.push_back(it->typeIcon);
        if (it->visual) allImages.push_back(it->visual);
        if (it->cardFrame) allImages.push_back(it->cardFrame);
        if (it->visualFrame) allImages.push_back(it->visualFrame);

        // Remove from tracking maps
        for (ImageObject* img : allImages) {
            origPos.erase(img);
            origSize.erase(img);
            origRot.erase(img);

            if (hoveredView == img) hoveredView = nullptr;
            if (selectedView == img) selectedView = nullptr;
        }

        views.erase(it);
    }

    layoutViews();
}

Card* Hand::FindCardByImage(ImageObject* img)
{
    for (auto& cv : views) {
        std::vector<ImageObject*> images = getAllImagesFromView(cv);
        if (std::find(images.begin(), images.end(), img) != images.end()) {
            return cv.cardData;
        }
    }
    return nullptr;
}

bool Hand::TrySelectAt(const glm::vec3& mouseWorld)
{
    ImageObject* hit = PeekAt(mouseWorld);
    if (hit) {
        selectedView = hit;
        return true;
    }
    return false;
}

void Hand::Deselect()
{
    selectedView = nullptr;
}