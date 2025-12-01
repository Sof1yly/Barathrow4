#include "Hand.h"
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

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

    const float W = 220.0f;
    const float H = 335.0f;
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
        ImageObject* card = views[i].image;
        if (!card) continue;

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

        card->SetPosition(glm::vec3(px, py, 0.0f));
        card->SetSize(W, -H); // -H to flip the upside-down texture
        card->SetRotate(rotDeg);

        origPos[card] = card->GetPosition();
        origSize[card] = card->GetSize();
        origRot[card] = card->GetRotate();
    }
}

void Hand::liftForHover(ImageObject* v)
{
    if (!v) return;

    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 135.0f;   // how high above fan
    const float HOVER_Z = 420.0f;   // above other cards
    const float HOVER_SCALE = 1.35f;     // change this to adjust hover size

    glm::vec3 newPos(basePos.x, basePos.y + HOVER_OFFSET_Y, HOVER_Z);

    glm::vec2 newSize(baseSize.x * HOVER_SCALE,baseSize.y * HOVER_SCALE);

    v->SetPosition(newPos);
    v->SetSize(newSize.x, newSize.y);
    v->SetRotate(0.0f); // straight up
}

void Hand::clearHover()
{
    if (!hoveredView) return;

    ImageObject* v = hoveredView;
    if (origPos.count(v))  v->SetPosition(origPos[v]);
    if (origSize.count(v)) v->SetSize(origSize[v].x, origSize[v].y);
    if (origRot.count(v))  v->SetRotate(origRot[v]);

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

        auto* img = new ImageObject();
        img->SetSize(220.0f, -335.0f);
        img->SetTexture("../Resource/Texture/cards/slayCardTest.png");
   

        objectsList.push_back(img);

        CardView cv;
        cv.cardData = data;
        cv.image = img;
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

        auto* img = new ImageObject();
        img->SetSize(220.0f, -335.0f);
        img->SetTexture("../Resource/Texture/cards/slayCardTest.png");

        objectsList.push_back(img);

        CardView cv;
        cv.cardData = data;
        cv.image = img;
        views.push_back(cv);
    }

    // re-fan whole hand
    layoutViews();
}

void Hand::Clear(std::vector<DrawableObject*>& objectsList)
{
    for (auto& v : views)
    {
        if (!v.image) {
            continue;
        }

        auto it = std::find(objectsList.begin(), objectsList.end(), v.image);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }

        delete v.image;
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
    if (hoveredView && hitTestCurrent(hoveredView, mouseWorld))
        return hoveredView;

    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        ImageObject* v = views[i].image;
        if (hitTestBase(v, mouseWorld))
            return v;
    }
    return nullptr;
}


void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging)
{
    if (isDragging)
    {
        return;
    }

    if (hoveredView)
    {
        if (hitTestCurrent(hoveredView, mouseWorld))
            return;
    }


    ImageObject* top = nullptr;
    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        ImageObject* v = views[i].image;
        if (hitTestBase(v, mouseWorld))
        {
            top = v;
            break;
        }
    }

    if (top == hoveredView) {
        return; // nothing changed
    }
        

    if (hoveredView)
        clearHover();

    if (top)
    {
        hoveredView = top;
        liftForHover(hoveredView);
    }
}


void Hand::RemoveView(ImageObject* view)
{
    if (!view) return;

    for (auto it = views.begin(); it != views.end(); ++it)
    {
        if (it->image == view)
        {
            views.erase(it);
            break;
        }
    }

    origPos.erase(view);
    origSize.erase(view);
    origRot.erase(view);

    if (hoveredView == view)  hoveredView = nullptr;
    if (selectedView == view) selectedView = nullptr;

    layoutViews();
}

Card* Hand::FindCardByImage(ImageObject* img)
{
    for (auto& v : views) {
         if (v.image == img){
			return v.cardData;
        }
    }
    return nullptr;
}


bool Hand::TrySelectAt(const glm::vec3& mouseWorld)
{
    ImageObject* hit = PeekAt(mouseWorld);
    if (!hit)
    {
        Deselect();
        return false;
    }

    if (selectedView == hit)
    {
        Deselect();
        return false;
    }

    if (selectedView && origPos.count(selectedView))
    {
        selectedView->SetPosition(origPos[selectedView]);
        selectedView->SetSize(origSize[selectedView].x, origSize[selectedView].y);
        selectedView->SetRotate(origRot[selectedView]);
    }

    selectedView = hit;
    liftForHover(selectedView);
    return true;
}

void Hand::Deselect()
{
    if (!selectedView) {
		return;
    }
    if (origPos.count(selectedView)) {
        selectedView->SetPosition(origPos[selectedView]);
    }    
    if (origSize.count(selectedView)) {
        selectedView->SetSize(origSize[selectedView].x, origSize[selectedView].y);
    }
    if (origRot.count(selectedView)) {
        selectedView->SetRotate(origRot[selectedView]);
    }
    selectedView = nullptr;
}
