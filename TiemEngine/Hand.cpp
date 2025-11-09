#include "Hand.h"
#include "GameObject.h"    
#include <algorithm>
#include <cmath>

static float PI = 3.1415926535f;
static float RAD2DEG = 180.0f / PI;

void Hand::CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList)
{
    if (cardCount <= 0) return;

    const int drawCount = cardCount;

    // layout params
    const float W = 220.0f, H = 335.0f;
    const float handY = -540.0f;

    const float baseRadius = 1000.0f;
    const float centerDrop = 120.0f;
    const float R = baseRadius + centerDrop;

    const float Cx = 0.0f, Cy = handY - R;
    const float sepFactor = 0.65f;  // spacing vs card width
    const float sink = 4.0f;        // tuck bottom slightly past rim


    float arcSpacing = W * sepFactor;                 // pixels along the arc
    float stepDeg = (arcSpacing / R) * RAD2DEG;       // angle per neighbor
    float spanDeg = (drawCount > 1) ? stepDeg * float(drawCount - 1) : 0.0f;
    float startDeg = -0.5f * spanDeg;

    for (int i = 0; i < drawCount; ++i) {
        float ang = (startDeg + i * stepDeg) * (PI / 180.0f);

        // circle rim position under the card
        float rimX = Cx + std::sinf(ang) * R;
        float rimY = Cy + std::cosf(ang) * R;

        // radial normal and tangent
        float nx = std::sinf(ang), ny = std::cosf(ang);
        float tx = ny, ty = -nx;

        // place sprite center slightly outside the rim
        float px = rimX + nx * (H * 0.5f - sink);
        float py = rimY + ny * (H * 0.5f - sink);

        float rotDeg = std::atan2f(ty, tx) * RAD2DEG; // tangent orientation

        auto* go = new GameObject();
        go->SetSize(W, H);
        go->SetPosition({ px, py, 0.0f });
        go->SetRotate(rotDeg);

        // quick tint to visualize order (optional)
        if (i % 3 == 0)      go->SetColor(5.0f, 0.0f, 0.0f);
        else if (i % 3 == 1) go->SetColor(0.0f, 5.0f, 0.0f);
        else                 go->SetColor(0.0f, 0.0f, 5.0f);

        views.push_back(go);
        origPos[go] = go->GetPosition();
        origSize[go] = go->GetSize();
        origRot[go] = go->GetRotate();

        objectsList.push_back(go);
    }


}

bool Hand::hitTest(GameObject* v, const glm::vec3& p) const
{
    // Simple AABB hit (ignores rotation; good enough for fanned hand)
    glm::vec3 pos = v->GetPosition();
    glm::vec2 sz = v->GetSize();
    float halfW = std::abs(sz.x) * 0.5f;
    float halfH = std::abs(sz.y) * 0.5f;

    return (p.x >= pos.x - halfW && p.x <= pos.x + halfW &&p.y >= pos.y - halfH && p.y <= pos.y + halfH);
}

void Hand::applySelectedVisual(GameObject* v)
{
    if (!v) return;

    glm::vec3 p = v->GetPosition();
    glm::vec2 s = v->GetSize();

    v->SetPosition(glm::vec3(p.x, p.y + 160.0f, 500.0f)); // Z up to draw on top
    v->SetSize(s.x * 1.25f, s.y * 1.25f);
    v->SetRotate(0.0f);

}

void Hand::restoreVisual(GameObject* v)
{
    if (!v) return;
    v->SetPosition(origPos[v]);
    glm::vec2 s = origSize[v];
    v->SetSize(s.x, s.y);
    v->SetRotate(origRot[v]);

}

bool Hand::TrySelectAt(const glm::vec3& mouseWorld)
{
    // Check top-most first: iterate from back of vector (last created on top)
    GameObject* hit = nullptr;
    for (int i = int(views.size()) - 1; i >= 0; --i) {
        if (hitTest(views[i], mouseWorld)) { hit = views[i]; break; }
    }

    // click on empty space: clear selection
    if (!hit) {
        Deselect();
        return false;
    }

    // toggle if clicking the same card
    if (selectedView == hit) {
        Deselect();
        return false;
    }

    // switch selection
    if (selectedView) restoreVisual(selectedView);
    selectedView = hit;
    applySelectedVisual(selectedView);
    return true;
}

void Hand::Deselect()
{
    if (selectedView) {
        restoreVisual(selectedView);
        selectedView = nullptr;
    }
}

GameObject* Hand::PeekAt(const glm::vec3& mouseWorld) const
{

    GameObject* hit = nullptr;
    for (int i = int(views.size()) - 1; i >= 0; --i) {
        if (hitTest(views[i], mouseWorld)) {
            hit = views[i];
            break;
        }
    }
    return hit;
}
