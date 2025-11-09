#include "Hand.h"
#include "GameObject.h"
#include <cmath>

static const float PI = 3.1415926535f;
static const float RAD2DEG = 180.0f / PI;

// ----------------- internal -----------------

bool Hand::hitTest(GameObject* v, const glm::vec3& p) const
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

    const float W = 220.0f; //card width
	const float H = 335.0f; //card height

    // base hand line at bottom
    const float handY = -620.0f;   // adjust to your screen
    const float baseRadius = 1000.0f;
    const float centerDrop = 120.0f;
    const float R = baseRadius + centerDrop;

    const float Cx = 0.0f;
    const float Cy = handY - R;

    const float sepFactor = 0.65f;
    const float sink = 4.0f;

    float arcSpacing = W * sepFactor;
    float stepDeg = (arcSpacing / R) * RAD2DEG;
    float spanDeg = (count > 1) ? stepDeg * (float)(count - 1) : 0.0f;
    float startDeg = -0.5f * spanDeg;

    for (int i = 0; i < count; ++i)
    {
        GameObject* go = views[i];
        if (!go) continue;

        float deg = startDeg + stepDeg * (float)i;
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

        go->SetPosition(glm::vec3(px, py, 0.0f));
        go->SetSize(W, H);
        go->SetRotate(rotDeg);

        origPos[go] = go->GetPosition();
        origSize[go] = go->GetSize();
        origRot[go] = go->GetRotate();
    }
}

void Hand::liftForHover(GameObject* v)
{
    if (!v) return;

    // Base transform from fan
    glm::vec3 basePos = origPos.count(v) ? origPos[v] : v->GetPosition();
    glm::vec2 baseSize = origSize.count(v) ? origSize[v] : v->GetSize();

    const float HOVER_OFFSET_Y = 260.0f;  // how far it pops up
    const float HOVER_Z = 350.0f;  // in front of other cards

    glm::vec3 newPos(basePos.x, basePos.y + HOVER_OFFSET_Y, HOVER_Z);

    v->SetPosition(newPos);
    v->SetSize(baseSize.x, baseSize.y);   // no scale
    v->SetRotate(0.0f);                   // straight vertical
}


void Hand::clearHover()
{
    if (!hoveredView) return;

    GameObject* v = hoveredView;

    if (origPos.find(v) != origPos.end())
        v->SetPosition(origPos[v]);

    if (origSize.find(v) != origSize.end())
        v->SetSize(origSize[v].x, origSize[v].y);

    if (origRot.find(v) != origRot.end())
        v->SetRotate(origRot[v]);

    hoveredView = nullptr;
}

// ----------------- public API -----------------

void Hand::CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList)
{
    // You can keep your logical deck; this just spawns visuals.
    deck.clear();
    views.clear();
    origPos.clear();
    origSize.clear();
    origRot.clear();
    hoveredView = nullptr;
    selectedView = nullptr;

    for (int i = 0; i < cardCount; ++i)
    {
        // This assumes GameObject is fine as a visual card.
        GameObject* go = new GameObject();
        go->SetColor(1.0f, 1.0f, 1.0f);   // actual texture set elsewhere
        objectsList.push_back(go);
        views.push_back(go);
    }

    layoutViews();
}

GameObject* Hand::PeekAt(const glm::vec3& mouseWorld)
{
    // search from topmost to bottom
    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        GameObject* v = views[i];
        if (hitTest(v, mouseWorld))
            return v;
    }
    return nullptr;
}

void Hand::UpdateHover(const glm::vec3& mouseWorld, bool isDragging)
{
    // During drag we don't want hover popping others
    if (isDragging) {
        if (hoveredView) {
            clearHover();
        }
        return;
    }

    // Find topmost under mouse
    GameObject* top = nullptr;
    for (int i = (int)views.size() - 1; i >= 0; --i)
    {
        GameObject* v = views[i];
        if (hitTest(v, mouseWorld)) {
            top = v;
            break;
        }
    }

    if (top == hoveredView) {
        // already previewing this one
        return;
    }

    // remove old hover
    if (hoveredView) {
        clearHover();
    }

    // apply new hover
    if (top) {
        hoveredView = top;
        liftForHover(hoveredView);
    }
}

void Hand::RemoveView(GameObject* view)
{
    if (!view) return;

    int idx = -1;
    for (int i = 0; i < (int)views.size(); ++i) {
        if (views[i] == view) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return;

    // keep logical deck aligned if you're using it
    if (idx < (int)deck.size()) {
        deck.erase(deck.begin() + idx);
    }

    views.erase(views.begin() + idx);

    origPos.erase(view);
    origSize.erase(view);
    origRot.erase(view);

    if (hoveredView == view) hoveredView = nullptr;
    if (selectedView == view) selectedView = nullptr;

    layoutViews();
}

bool Hand::TrySelectAt(const glm::vec3& mouseWorld)
{
    GameObject* v = PeekAt(mouseWorld);
    if (!v) {
        Deselect();
        return false;
    }
    selectedView = v;
    return true;
}

void Hand::Deselect()
{
    selectedView = nullptr;
    // no auto-reset of transforms here; hover/layout handles visuals
}
