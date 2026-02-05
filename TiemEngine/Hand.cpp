#include "Hand.h"
#include "Action.h"
#include "MoveAction.h"
#include "AttackAction.h"
#include "TextObject.h"
#include <iostream>
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

//rarity
static std::string getCardFrameName(const std::string& rarityCode)
{
    if (rarityCode == "sta") return "BG_Frame/card_fr_gy.png";
    if (rarityCode == "com") return "BG_Frame/card_fr_gn.png";
    if (rarityCode == "rar") return "BG_Frame/card_fr_br.png";
    if (rarityCode == "leg") return "BG_Frame/card_fr_or.png";
    if (rarityCode == "spc") return "BG_Frame/card_fr_pk.png";
    return "BG_Frame/card_fr_gy.png";
}

//type
static std::string getTypeIconName(const std::string& typeCode)
{
    if (typeCode == "atk") return "BG_Type/card_ty_atk.png";
    if (typeCode == "mov") return "BG_Type/card_ty_mv.png";
    if (typeCode == "eng") return "BG_Type/card_ty_er.png";
    if (typeCode == "buf") return "BG_Type/card_ty_bf.png";
    if (typeCode == "dbf") return "BG_Type/card_ty_db.png";
    return "BG_Type/card_ty_atk.png";
}

// level
static std::string getStarOverlayName(int level)
{
    switch (level) {
    case 1: return "BG_Stars/star_gd1.png";
    case 2: return "BG_Stars/star_gd2.png";
    case 3: return "BG_Stars/star_gd3.png";
    default: return "";
    }
}

static std::string getBackgroundName()
{
    return "BG_card/card_bg.png";
}

std::vector<DrawableObject*> Hand::getAllImagesFromView(const CardView& cv) const
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
        std::vector<DrawableObject*> allLayers = getAllImagesFromView(cv);

        if (allLayers.empty()) continue;

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
        
  
        float baseZ = 100.0f + (i * 10.0f);
        float rotDeg = std::atan2f(ty, tx) * RAD2DEG;

        int layerIndex = 0;
        for (DrawableObject* layer : allLayers) {
            if (!layer) continue;

            float layerZ = baseZ + (layerIndex * 0.1f);
            
            if (TextObject* textObj = dynamic_cast<TextObject*>(layer)) {
                float textOffsetX = 45.0f;   // Move right
                float textOffsetY = 135.0f; // Move up (higher value = higher position)
				
                // Apply rotation to the offset vector
                float angleRad = rotDeg * (PI / 180.0f);
                float rotatedOffsetX = textOffsetX * std::cosf(angleRad) - textOffsetY * std::sinf(angleRad);
                float rotatedOffsetY = textOffsetX * std::sinf(angleRad) + textOffsetY * std::cosf(angleRad);
                
                textObj->SetPosition(glm::vec3(px + rotatedOffsetX, py + rotatedOffsetY, layerZ));
                textObj->SetRotate(rotDeg);
            }
            else {
                layer->SetPosition(glm::vec3(px, py, layerZ));
                layer->SetSize(W, -H);
                layer->SetRotate(rotDeg);
            }

            // Store for ImageObject (for hit testing)
            if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
                origPos[img] = layer->GetPosition();
                origSize[img] = layer->GetSize();
                origRot[img] = layer->GetRotate();
            }
            
            layerIndex++;
        }
    }
}

void Hand::liftForHover(ImageObject* v, std::vector<DrawableObject*>& objectsList)
{
    if (!v) return;

    CardView* targetView = nullptr;
    for (auto& cv : views) {
        std::vector<DrawableObject*> layers = getAllImagesFromView(cv);
        for (DrawableObject* layer : layers) {
            if (layer == v) {
                targetView = &cv;
                break;
            }
        }
        if (targetView) break;
    }

    if (!targetView) return;

    std::vector<DrawableObject*> allLayers = getAllImagesFromView(*targetView);

    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 120.0f;
    const float HOVER_Z_BASE = 1000.0f;
    const float HOVER_SCALE = 1.3f;

    // SAVE original indices before removing
    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
                origIndices[img] = std::distance(objectsList.begin(), it);
            }
        }
    }

    // REMOVE all layers from objectsList first
    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }

    int layerIndex = 0;
    for (DrawableObject* layer : allLayers) {
        if (!layer) continue;
        
        float layerZ = HOVER_Z_BASE + (layerIndex * 0.1f);
        
        if (TextObject* textObj = dynamic_cast<TextObject*>(layer)) {

            float textOffsetX = 44.0f; // Move right 
            float textOffsetY = 125.0f * HOVER_SCALE; // Move up
            textObj->SetPosition(glm::vec3(basePos.x + textOffsetX, basePos.y + HOVER_OFFSET_Y + textOffsetY, layerZ));
            textObj->SetRotate(0.0f);
        }
        else {
            layer->SetPosition(glm::vec3(basePos.x, basePos.y + HOVER_OFFSET_Y, layerZ));
            layer->SetSize(baseSize.x * HOVER_SCALE, baseSize.y * HOVER_SCALE);
            layer->SetRotate(0.0f);
        }
        
        // Add back to END of objectsList
        objectsList.push_back(layer);
        
        layerIndex++;
    }
}

void Hand::clearHover(std::vector<DrawableObject*>& objectsList)
{
    if (!hoveredView) return;

    CardView* targetView = nullptr;
    int cardIndex = -1;
    
    // Find which card is being hovered and its index
    for (size_t i = 0; i < views.size(); ++i) {
        std::vector<DrawableObject*> layers = getAllImagesFromView(views[i]);
        for (DrawableObject* layer : layers) {
            ImageObject* img = dynamic_cast<ImageObject*>(layer);
            if (img == hoveredView) {
                targetView = &views[i];
                cardIndex = static_cast<int>(i);
                break;
            }
        }
        if (targetView) break;
    }

    if (!targetView) {
        hoveredView = nullptr;
        return;
    }

    std::vector<DrawableObject*> allLayers = getAllImagesFromView(*targetView);
    
    // REMOVE all layers of the hovered card from objectsList
    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }
    
    // This ensures the card goes back to its original position in the rendering order
    size_t insertionPoint = objectsList.size(); 
    
    if (cardIndex < static_cast<int>(views.size()) - 1) {
        // There are cards after this one, find where their layers start
        for (size_t nextCardIdx = cardIndex + 1; nextCardIdx < views.size(); ++nextCardIdx) {
            std::vector<DrawableObject*> nextCardLayers = getAllImagesFromView(views[nextCardIdx]);
            if (!nextCardLayers.empty()) {
                // Find the first layer of the next card in objectsList
                for (size_t i = 0; i < objectsList.size(); ++i) {
                    if (objectsList[i] == nextCardLayers[0]) {
                        insertionPoint = i;
                        break;
                    }
                }
                if (insertionPoint < objectsList.size()) {
                    break;
                }
            }
        }
    }
    
    // Insert all layers at the calculated position (in order: background, star, type, visual, frame, text)
    for (DrawableObject* layer : allLayers) {
        objectsList.insert(objectsList.begin() + insertionPoint, layer);
        insertionPoint++; // Move insertion point forward for next layer
    }
    
    // Clear saved indices
    for (DrawableObject* layer : allLayers) {
        if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
            origIndices.erase(img);
        }
    }

    hoveredView = nullptr;
    
    // Recalculate all card positions (including text) to restore fan layout
    layoutViews();
    

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
        std::vector<DrawableObject*> allLayers = data->GetAllLayers();
        for (DrawableObject* layer : allLayers) {
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
        std::vector<DrawableObject*> allLayers = data->GetAllLayers();
        for (DrawableObject* layer : allLayers) {
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
            std::vector<DrawableObject*> layers = cv.cardData->GetAllLayers();
            for (DrawableObject* layer : layers) {
                if (dynamic_cast<ImageObject*>(layer) == view) {
                    return true;
                }
            }
            return false;
        });

    if (it != views.end()) {
        std::vector<DrawableObject*> allLayers = it->cardData->GetAllLayers();

        for (DrawableObject* layer : allLayers) {
            // Remove from objectsList
            auto objIt = std::find(objectsList.begin(), objectsList.end(), layer);
            if (objIt != objectsList.end()) {
                objectsList.erase(objIt);
            }
            
            // Clean up tracking (only for ImageObjects)
            if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
                origPos.erase(img);
                origSize.erase(img);
                origRot.erase(img);
                origIndices.erase(img);

                if (hoveredView == img) hoveredView = nullptr;
                if (selectedView == img) selectedView = nullptr;
            }
        }

        views.erase(it);
    }

    layoutViews();
}

void Hand::Clear(std::vector<DrawableObject*>& objectsList)
{
    for (auto& cv : views)
    {
        if (!cv.cardData) continue;
        
        std::vector<DrawableObject*> allLayers = cv.cardData->GetAllLayers();

        for (DrawableObject* layer : allLayers) {
            if (!layer) continue;

            auto it = std::find(objectsList.begin(), objectsList.end(), layer);
            if (it != objectsList.end()) {
                objectsList.erase(it);
            }
        }
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
        std::vector<DrawableObject*> layers = getAllImagesFromView(cv);
        for (DrawableObject* layer : layers) {
            if (dynamic_cast<ImageObject*>(layer) == img) {
                return cv.cardData;
            }
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
        std::vector<DrawableObject*> layers = getAllImagesFromView(cv);
        for (DrawableObject* layer : layers) {
            if (dynamic_cast<ImageObject*>(layer) == anyLayer) {
                // Convert DrawableObject* vector to ImageObject* vector
                std::vector<ImageObject*> imageLayers;
                for (DrawableObject* l : layers) {
                    if (ImageObject* img = dynamic_cast<ImageObject*>(l)) {
                        imageLayers.push_back(img);
                    }
                }
                return imageLayers;
            }
        }
    }
    
    return {};
}