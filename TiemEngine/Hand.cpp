#include "Hand.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include <iostream>
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

// Helper function to get card frame filename based on rarity
static std::string getCardFrameName(const std::string& rarityCode)
{
    if (rarityCode == "sta") return "BG_Frame/card_fr_gy.png";
    if (rarityCode == "com") return "BG_Frame/card_fr_gn.png";
    if (rarityCode == "rar") return "BG_Frame/card_fr_br.png";
    if (rarityCode == "leg") return "BG_Frame/card_fr_or.png";
    if (rarityCode == "spc") return "BG_Frame/card_fr_pk.png";
    return "BG_Frame/card_fr_gy.png";
}

// Helper function to get type icon filename based on type
static std::string getTypeIconName(const std::string& typeCode)
{
    if (typeCode == "atk") return "BG_Type/card_ty_atk.png";
    if (typeCode == "mov") return "BG_Type/card_ty_mv.png";
    if (typeCode == "eng") return "BG_Type/card_ty_er.png";
    if (typeCode == "buf") return "BG_Type/card_ty_bf.png";
    if (typeCode == "dbf") return "BG_Type/card_ty_db.png";
    return "BG_Type/card_ty_atk.png";
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
    if (!cv.cardData) return {};
    return cv.cardData->GetAllLayers();
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
    const float handY = -610.0f;
    const float baseRadius = 2100.0f;
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
        
        // Base Z for each card increases left to right
        // Each layer within a card also increases slightly
        float baseZ = 100.0f + (i * 20.0f);

        float rotDeg = std::atan2f(ty, tx) * RAD2DEG;

        // Apply transformation to all layers with increasing Z per layer
        int layerIndex = 0;
        for (ImageObject* img : allImages) {
            if (!img) continue;

            float layerZ = baseZ + (layerIndex * 0.1f); // Each layer slightly above previous
            
            img->SetPosition(glm::vec3(px, py, layerZ));
            img->SetSize(W, -H);
            img->SetRotate(rotDeg);

            origPos[img] = img->GetPosition();
            origSize[img] = img->GetSize();
            origRot[img] = img->GetRotate();
            
            layerIndex++;
        }
    }
}

void Hand::liftForHover(ImageObject* v, std::vector<DrawableObject*>& objectsList)
{
    if (!v) return;

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

    std::vector<ImageObject*> allImages = getAllImagesFromView(*targetView);

    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 120.0f;
    const float HOVER_Z_BASE = 1000.0f;
    const float HOVER_SCALE = 1.3f;

    // SAVE original indices before removing
    for (ImageObject* img : allImages) {
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) {
            origIndices[img] = std::distance(objectsList.begin(), it);
        }
    }

    // REMOVE all layers from objectsList first
    for (ImageObject* img : allImages) {
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }

    // Apply hover effect and RE-ADD to END of objectsList (renders on top)
    int layerIndex = 0;
    for (ImageObject* img : allImages) {
        if (!img) continue;
        
        float layerZ = HOVER_Z_BASE + (layerIndex * 0.1f);
        
        img->SetPosition(glm::vec3(basePos.x, basePos.y + HOVER_OFFSET_Y, layerZ));
        img->SetSize(baseSize.x * HOVER_SCALE, baseSize.y * HOVER_SCALE);
        img->SetRotate(0.0f);
        
        // Add back to END of objectsList
        objectsList.push_back(img);
        
        layerIndex++;
    }
}

void Hand::clearHover(std::vector<DrawableObject*>& objectsList)
{
    if (!hoveredView) return;

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

    std::vector<ImageObject*> allImages = getAllImagesFromView(*targetView);
    
    // REMOVE from objectsList
    for (ImageObject* img : allImages) {
        auto it = std::find(objectsList.begin(), objectsList.end(), img);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }
    
    // Restore original properties
    for (ImageObject* img : allImages) {
        if (!img) continue;
        
        if (origPos.count(img)) {
            img->SetPosition(origPos[img]);
        }
        
        if (origSize.count(img)) {
            img->SetSize(origSize[img].x, origSize[img].y);
        }
        
        if (origRot.count(img)) {
            img->SetRotate(origRot[img]);
        }
    }
    
    // Sort by original index (lowest first) to maintain proper order
    std::vector<std::pair<ImageObject*, size_t>> imagesToRestore;
    for (ImageObject* img : allImages) {
        if (origIndices.count(img)) {
            imagesToRestore.push_back({img, origIndices[img]});
        }
    }
    
    // Sort by original index
    std::sort(imagesToRestore.begin(), imagesToRestore.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    
    // Insert back at original positions (adjusted for already removed items)
    for (const auto& pair : imagesToRestore) {
        ImageObject* img = pair.first;
        size_t originalIndex = pair.second;
        
        // Clamp to valid range
        size_t insertPos = std::min(originalIndex, objectsList.size());
        
        objectsList.insert(objectsList.begin() + insertPos, img);
    }
    
    // Clear saved indices
    for (ImageObject* img : allImages) {
        origIndices.erase(img);
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
        if (!data) continue;

        // Create visuals in Card if not already created
        if (!data->HasVisuals()) {
            data->CreateVisuals();
        }

        CardView cv;
        cv.cardData = data;
        cv.background = data->GetBackground();
        cv.starOverlay = data->GetStarOverlay();
        cv.typeIcon = data->GetTypeIcon();
        cv.visual = data->GetVisual();
        cv.cardFrame = data->GetCardFrame();

        // Add all layers to objectsList
        std::vector<ImageObject*> allLayers = data->GetAllLayers();
        for (ImageObject* layer : allLayers) {
            if (layer) {
                objectsList.push_back(layer);
            }
        }

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

        // Create visuals in Card if not already created
        if (!data->HasVisuals()) {
            data->CreateVisuals();
        }

        CardView cv;
        cv.cardData = data;
        cv.background = data->GetBackground();
        cv.starOverlay = data->GetStarOverlay();
        cv.typeIcon = data->GetTypeIcon();
        cv.visual = data->GetVisual();
        cv.cardFrame = data->GetCardFrame();

        // Add all layers to objectsList
        std::vector<ImageObject*> allLayers = data->GetAllLayers();
        for (ImageObject* layer : allLayers) {
            if (layer) {
                objectsList.push_back(layer);
            }
        }

        views.push_back(cv);
    }

    layoutViews();
}

void Hand::RemoveView(ImageObject* view, std::vector<DrawableObject*>& objectsList)
{
    if (!view) return;

    auto it = std::find_if(views.begin(), views.end(),
        [view](const CardView& cv) {
            if (!cv.cardData) return false;
            std::vector<ImageObject*> images = cv.cardData->GetAllLayers();
            return std::find(images.begin(), images.end(), view) != images.end();
        });

    if (it != views.end()) {
        std::vector<ImageObject*> allImages = it->cardData->GetAllLayers();

        for (ImageObject* img : allImages) {
            // Remove from objectsList
            auto objIt = std::find(objectsList.begin(), objectsList.end(), img);
            if (objIt != objectsList.end()) {
                objectsList.erase(objIt);
            }
            
            // Clean up tracking
            origPos.erase(img);
            origSize.erase(img);
            origRot.erase(img);
            origIndices.erase(img);

            if (hoveredView == img) hoveredView = nullptr;
            if (selectedView == img) selectedView = nullptr;
        }
        
        // Note: We don't delete the ImageObjects here anymore
        // They are owned by Card and will be cleaned up when Card is destroyed

        views.erase(it);
    }

    layoutViews();
}

void Hand::Clear(std::vector<DrawableObject*>& objectsList)
{
    for (auto& cv : views)
    {
        if (!cv.cardData) continue;
        
        std::vector<ImageObject*> allImages = cv.cardData->GetAllLayers();

        for (ImageObject* img : allImages) {
            if (!img) continue;

            auto it = std::find(objectsList.begin(), objectsList.end(), img);
            if (it != objectsList.end()) {
                objectsList.erase(it);
            }
        }
        
        // Note: We don't delete the ImageObjects here anymore
        // They are owned by Card and will be cleaned up when Card is destroyed
    }

    views.clear();
    hoveredView = nullptr;
    selectedView = nullptr;
    origPos.clear();
    origSize.clear();
    origRot.clear();
    origIndices.clear();
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

void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging, std::vector<DrawableObject*>& objectsList)
{
    if (isDragging) {
        if (hoveredView) {
            clearHover(objectsList);
        }
        return;
    }

    ImageObject* hit = PeekAt(mouseWorld);

    if (hit == hoveredView) {
        return;
    }

    if (hoveredView) {
        clearHover(objectsList);
    }

    if (hit) {
        hoveredView = hit;
        liftForHover(hit, objectsList);

        Card* cardData = FindCardByImage(hit);
        if (cardData) {
            cout << "Hover card: " << cardData->getName();

            const auto& actions = cardData->getActions();
            for (Action* a : actions) {
                if(auto* atk = dynamic_cast<AttackAction*>(a)) {
                    cout << " Attack: " << atk->getValue();
                }
                else if(auto* mv = dynamic_cast<MoveAction*>(a)) {
                    cout << " Move: " << mv->getValue();
                }
            }
            cout << endl;
        }
    }
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

std::vector<ImageObject*> Hand::GetAllLayersForCard(ImageObject* anyLayer)
{
    if (!anyLayer) return {};
    
    // Find the CardView that contains this layer
    for (auto& cv : views) {
        std::vector<ImageObject*> images = getAllImagesFromView(cv);
        if (std::find(images.begin(), images.end(), anyLayer) != images.end()) {
            return images;
        }
    }
    
    return {};
}