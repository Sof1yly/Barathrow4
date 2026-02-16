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
static string getCardFrameName(const string& rarityCode)
{
    if (rarityCode == "sta") return "BG_Frame/card_fr_gy.png";
    if (rarityCode == "com") return "BG_Frame/card_fr_gn.png";
    if (rarityCode == "rar") return "BG_Frame/card_fr_br.png";
    if (rarityCode == "leg") return "BG_Frame/card_fr_or.png";
    if (rarityCode == "spc") return "BG_Frame/card_fr_pk.png";
    return "BG_Frame/card_fr_gy.png";
}

//type
static string getTypeIconName(const string& typeCode)
{
    if (typeCode == "atk") return "BG_Type/card_ty_atk.png";
    if (typeCode == "mov") return "BG_Type/card_ty_mv.png";
    if (typeCode == "eng") return "BG_Type/card_ty_er.png";
    if (typeCode == "buf") return "BG_Type/card_ty_bf.png";
    if (typeCode == "dbf") return "BG_Type/card_ty_db.png";
    return "BG_Type/card_ty_atk.png";
}

// level
static string getStarOverlayName(int level)
{
    switch (level) {
    case 1: return "BG_Stars/star_gd1.png";
    case 2: return "BG_Stars/star_gd2.png";
    case 3: return "BG_Stars/star_gd3.png";
    default: return "";
    }
}

static string getBackgroundName()
{
    return "BG_card/card_bg.png";
}

vector<DrawableObject*> Hand::getAllImagesFromView(const CardView& cv) const
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

    return (p.x >= pos.x - halfW && p.x <= pos.x + halfW &&p.y >= pos.y - halfH && p.y <= pos.y + halfH);
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
        vector<DrawableObject*> allLayers = getAllImagesFromView(cv);

        if (allLayers.empty()) continue;

        // Check if this card is currently being dragged
        bool isDraggedCard = false;
        if (draggingView) {
            for (DrawableObject* layer : allLayers) {
                if (dynamic_cast<ImageObject*>(layer) == draggingView) {
                    isDraggedCard = true;
                    break;
                }
            }
        }

        float deg = startDeg + stepDeg * i;
        float ang = deg * (PI / 180.0f);

        float rimX = Cx + sinf(ang) * R;
        float rimY = Cy + cosf(ang) * R;

        float nx = sinf(ang);
        float ny = cosf(ang);
        float tx = ny;
        float ty = -nx;

        float px = rimX + nx * (H * 0.5f - sink);
        float py = rimY + ny * (H * 0.5f - sink);
        
  
        float baseZ = 100.0f + (i * 10.0f);
        float rotDeg = atan2f(ty, tx) * RAD2DEG;

        int layerIndex = 0;
        int textObjectCount = 0;
        for (DrawableObject* layer : allLayers) {
            if (!layer) continue;

            float layerZ = baseZ + (layerIndex * 0.1f);
            
            // Skip repositioning dragged card layers, but still store origPos
            if (!isDraggedCard) {
                if (TextObject* textObj = dynamic_cast<TextObject*>(layer)) {
                    textObjectCount++;
                    float textOffsetX = 35.0f;
                    float textOffsetY;
                    
                    if (textObjectCount == 1) {
                        textOffsetY = 130.0f; // Name position (top)
                    } else {
                        textOffsetY = -80.0f; // Description position (below main visual)
                    }
				    
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
            }

            // Store for ImageObject (always, even for dragged card)
            if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
                origPos[img] = glm::vec3(px, py, layerZ);
                origSize[img] = glm::vec2(W, -H);
                origRot[img] = rotDeg;
            }
            
            layerIndex++;
        }
    }
}

void Hand::liftForHover(ImageObject* v, vector<DrawableObject*>& objectsList)
{
    if (!v) return;

    CardView* targetView = nullptr;
    for (auto& cv : views) {
        vector<DrawableObject*> layers = getAllImagesFromView(cv);
        for (DrawableObject* layer : layers) {
            if (layer == v) {
                targetView = &cv;
                break;
            }
        }
        if (targetView) break;
    }

    if (!targetView) return;

    vector<DrawableObject*> allLayers = getAllImagesFromView(*targetView);

    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 120.0f;
    const float HOVER_Z_BASE = 1000.0f;
    const float HOVER_SCALE = 1.3f;

    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            if (ImageObject* img = dynamic_cast<ImageObject*>(layer)) {
                origIndices[img] = std::distance(objectsList.begin(), it);
            }
        }
    }

    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }

    int layerIndex = 0;
    int textObjectCount = 0;
    for (DrawableObject* layer : allLayers) {
        if (!layer) continue;
        
        float layerZ = HOVER_Z_BASE + (layerIndex * 0.1f);
        
        if (TextObject* textObj = dynamic_cast<TextObject*>(layer)) {
            textObjectCount++;
            float textOffsetX = 22.5f * HOVER_SCALE;
            float textOffsetY;
            
            if (textObjectCount == 1) {
                textOffsetY = 135.0f * HOVER_SCALE; // Name position (top) 135
            } else {
                textOffsetY = -80.0f * HOVER_SCALE; // Description position (below main visual)
            }
            
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

void Hand::clearHover(vector<DrawableObject*>& objectsList)
{
    if (!hoveredView) return;

    CardView* targetView = nullptr;
    int cardIndex = -1;
    
    // Find which card is being hovered and its index
    for (size_t i = 0; i < views.size(); ++i) {
        vector<DrawableObject*> layers = getAllImagesFromView(views[i]);
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

    vector<DrawableObject*> allLayers = getAllImagesFromView(*targetView);
    
    // REMOVE all layers of the hovered card from objectsList
    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }
    
    size_t insertionPoint = objectsList.size(); 
    
    if (cardIndex < static_cast<int>(views.size()) - 1) {
        // There are cards after this one, find where their layers start
        for (size_t nextCardIdx = cardIndex + 1; nextCardIdx < views.size(); ++nextCardIdx) {
            // Skip the dragged card when looking for insertion point
            if (draggingView) {
                std::vector<DrawableObject*> checkLayers = getAllImagesFromView(views[nextCardIdx]);
                bool isNextDragged = false;
                for (DrawableObject* layer : checkLayers) {
                    if (dynamic_cast<ImageObject*>(layer) == draggingView) {
                        isNextDragged = true;
                        break;
                    }
                }
                if (isNextDragged) continue;
            }

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
    
    // Insert all layers at the calculated position
    for (DrawableObject* layer : allLayers) {
        objectsList.insert(objectsList.begin() + insertionPoint, layer);
        insertionPoint++; 
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

void Hand::CreateVisualHand(int cardCount,vector<DrawableObject*>& objectsList,const vector<Card*>& cardData)
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
        vector<DrawableObject*> allLayers = data->GetAllLayers();
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

void Hand::AddCards(const vector<Card*>& cardsToAdd,
    vector<DrawableObject*>& objectsList)
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
        vector<DrawableObject*> allLayers = data->GetAllLayers();
        for (DrawableObject* layer : allLayers) {
            if (layer) {
                objectsList.push_back(layer);
            }
        }

        views.push_back(cv);
    }

    layoutViews();
}

void Hand::RemoveView(ImageObject* view, vector<DrawableObject*>& objectsList)
{
    if (!view) return;

    auto it = find_if(views.begin(), views.end(),
        [view](const CardView& cv) {
            if (!cv.cardData) return false;
            vector<DrawableObject*> layers = cv.cardData->GetAllLayers();
            for (DrawableObject* layer : layers) {
                if (dynamic_cast<ImageObject*>(layer) == view) {
                    return true;
                }
            }
            return false;
        });

    if (it != views.end()) {
        vector<DrawableObject*> allLayers = it->cardData->GetAllLayers();

        for (DrawableObject* layer : allLayers) {
            // Remove from objectsList
            auto objIt = find(objectsList.begin(), objectsList.end(), layer);
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

void Hand::Clear(vector<DrawableObject*>& objectsList)
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

vector<Card*> Hand::CollectAllCardData() const
{
    vector<Card*> out;
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

void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging, vector<DrawableObject*>& objectsList)
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
        vector<DrawableObject*> layers = getAllImagesFromView(cv);
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

void Hand::RestoreLayout(ImageObject* card, vector<DrawableObject*>& objectsList)
{
    if (!card) return;

    // Find this card's index and layers
    int cardIndex = -1;
    vector<DrawableObject*> allLayers;

    for (size_t i = 0; i < views.size(); ++i) {
        vector<DrawableObject*> layers = getAllImagesFromView(views[i]);
        for (DrawableObject* layer : layers) {
            if (dynamic_cast<ImageObject*>(layer) == card) {
                cardIndex = static_cast<int>(i);
                allLayers = layers;
                break;
            }
        }
        if (cardIndex >= 0) break;
    }

    if (cardIndex < 0 || allLayers.empty()) return;

    // Remove layers from current position in objectsList
    for (DrawableObject* layer : allLayers) {
        auto it = std::find(objectsList.begin(), objectsList.end(), layer);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }
    }

    // Find correct insertion point (before the next card's layers)
    size_t insertionPoint = objectsList.size();

    if (cardIndex < static_cast<int>(views.size()) - 1) {
        for (size_t nextCardIdx = cardIndex + 1; nextCardIdx < views.size(); ++nextCardIdx) {
            std::vector<DrawableObject*> nextCardLayers = getAllImagesFromView(views[nextCardIdx]);
            if (!nextCardLayers.empty()) {
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

    // Reinsert at correct position
    for (DrawableObject* layer : allLayers) {
        objectsList.insert(objectsList.begin() + insertionPoint, layer);
        insertionPoint++;
    }

    // Recalculate all positions
    layoutViews();
}

vector<DrawableObject*> Hand::GetAllLayersForCard(ImageObject* anyLayer)
{
    if (!anyLayer) return {};
    
    // Find the CardView that contains this layer
    for (auto& cv : views) {
        std::vector<DrawableObject*> layers = getAllImagesFromView(cv);
        for (DrawableObject* layer : layers) {
            if (dynamic_cast<ImageObject*>(layer) == anyLayer) {
                return layers;
            }
        }
    }
    
    return {};
}