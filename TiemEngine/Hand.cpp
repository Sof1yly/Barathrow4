#include "Hand.h"
#include "GameObject.h"    
#include <algorithm>
#include <cmath>

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
    const float PI = 3.1415926535f, RAD2DEG = 180.0f / PI;

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

        objectsList.push_back(go);
    }
}

