#include "Hand.h"
#include "ImageObject.h"
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

bool Hand::hitTest(ImageObject* v, const glm::vec3& p) const
{
    if (!v) return false;

    glm::vec3 pos = v->GetPosition();
    glm::vec2 sz = v->GetSize();

    float halfW = fabs(sz.x) * 0.5f;
    float halfH = fabs(sz.y) * 0.5f;

    return (p.x >= pos.x - halfW && p.x <= pos.x + halfW &&
        p.y >= pos.y - halfH && p.y <= pos.y + halfH);
}

void Hand::layoutViews()
{
    int count = (int)views.size();
    if (count <= 0) return;

    const float W = 220.0f;
    const float H = 335.0f;
    const float handY = -610.0f; // position of hand base
    const float baseRadius = 2100.0f; // bigger = flatter, smaller more curverd 
    const float R = baseRadius + 120.0f;
    const float Cx = 0.0f;
    const float Cy = handY - R;
    const float sepFactor = 0.65f;
    const float sink = 4.0f;

    float arcSpacing = W * sepFactor;
    float stepDeg = (arcSpacing / R) * RAD2DEG;
    float spanDeg = (count > 1) ? stepDeg * (count - 1) : 0.0f;
    float startDeg = -0.5f * spanDeg;

    for (int i = 0; i < count; ++i)
    {
        ImageObject* card = views[i];
        if (!card) continue;

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

        float rotDeg = atan2f(ty, tx) * RAD2DEG;

        card->SetPosition(glm::vec3(px, py, 0.0f));
        card->SetSize(W, -H);
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

    const float HOVER_OFFSET_Y = 230.0f;
    const float HOVER_Z = 350.0f;

    glm::vec3 newPos(basePos.x, basePos.y + HOVER_OFFSET_Y, HOVER_Z);

    v->SetPosition(newPos);
    v->SetSize(baseSize.x, baseSize.y); // same size
    v->SetRotate(0.0f);
}

void Hand::clearHover()
{
    if (!hoveredView) return;

    ImageObject* v = hoveredView;
    if (origPos.count(v)) v->SetPosition(origPos[v]);
    if (origSize.count(v)) v->SetSize(origSize[v].x, origSize[v].y);
    if (origRot.count(v)) v->SetRotate(origRot[v]);

    hoveredView = nullptr;
}

void Hand::CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList)
{
    deck.clear();
    views.clear();
    origPos.clear();
    origSize.clear();
    origRot.clear();
    hoveredView = nullptr;
    selectedView = nullptr;

    for (int i = 0; i < cardCount; ++i)
    {
        ImageObject* card = new ImageObject();
        card->SetSize(220.0f, -335.0f);
        card->SetTexture("../Resource/Texture/cards/slayCardTest.png"); 
        views.push_back(card);
        objectsList.push_back(card);
    }

    layoutViews();
}

ImageObject* Hand::PeekAt(const glm::vec3& mouseWorld)
{
    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        ImageObject* v = views[i];
        if (hitTest(v, mouseWorld))
            return v;
    }
    return nullptr;
}

void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging)
{
    if (isDragging)
    {
        if (hoveredView) clearHover();
        return;
    }

    ImageObject* top = nullptr;
    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        ImageObject* v = views[i];
        if (hitTest(v, mouseWorld)) {
            top = v;
            break;
        }
    }

    if (top == hoveredView) return;
    if (hoveredView) clearHover();
    if (top) {
        hoveredView = top;
        liftForHover(hoveredView);
    }
}

void Hand::RemoveView(ImageObject* view)
{
    if (!view) return;

    int idx = -1;
    for (int i = 0; i < (int)views.size(); ++i) {
        if (views[i] == view) { idx = i; break; }
    }
    if (idx == -1) return;

    if (idx < (int)deck.size()) deck.erase(deck.begin() + idx);
    views.erase(views.begin() + idx);

    origPos.erase(view);
    origSize.erase(view);
    origRot.erase(view);

    if (hoveredView == view) hoveredView = nullptr;
    if (selectedView == view) selectedView = nullptr;

    layoutViews();
}
