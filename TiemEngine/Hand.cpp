#include "Hand.h"
#include "GameObject.h"
#include <cmath>
#include <algorithm>

static constexpr float W = 220.0f;
static constexpr float H = 335.0f;
static constexpr float HAND_Y = -540.0f;
static constexpr float BASE_R = 1000.0f;
static constexpr float CENTER_DROP = 120.0f;
static constexpr float SEP_FACTOR = 0.65f;   // 0.5–0.7 hand spread
static constexpr float SINK = 4.0f;
static constexpr float PI = 3.1415926535f;
static constexpr float RAD2DEG = 180.0f / PI;

void Hand::CreateVisualHand(int cardCount, std::vector<DrawableObject*>& objectsList)
{
    cardsIn.clear();
    if (cardCount <= 0) return;

    cardsIn.resize(cardCount);

    for (int i = 0; i < cardCount; ++i) {
        float x, y, rotDeg;
        computeLayout(i, cardCount, x, y, rotDeg);

        auto* go = new GameObject();
        go->SetSize(W, H);
        go->SetPosition({ x, y, 0.0f });
        go->SetRotate(rotDeg);
        // visual tint (optional)
        if (i % 3 == 0)      go->SetColor(5.0f, 0.0f, 0.0f);
        else if (i % 3 == 1) go->SetColor(0.0f, 5.0f, 0.0f);
        else                 go->SetColor(0.0f, 0.0f, 5.0f);

        objectsList.push_back(go);

        cardsIn[i].view = go;
        cardsIn[i].homeX = x;
        cardsIn[i].homeY = y;
        cardsIn[i].homeRotDeg = rotDeg;
    }
}

bool Hand::Owns(GameObject* view) const {
    return std::any_of(cardsIn.begin(), cardsIn.end(),
        [&](const CardHand& c) { return c.view == view; });
}

void Hand::SnapBack(GameObject* view) {
    if (!view) return;
    auto it = std::find_if(cardsIn.begin(), cardsIn.end(),
        [&](const CardHand& c) { return c.view == view; });
    if (it != cardsIn.end()) applyHome(*it);  // homeX/homeY/homeRotDeg -> view
}

void Hand::UseCardAndRefan(GameObject* view) {
    if (!view) return;
    auto it = std::find_if(cardsIn.begin(), cardsIn.end(),
        [&](const CardHand& I) { return I.view == view; });
    if (it == cardsIn.end()) return;

    // We do NOT delete the view here — Level can decide to re-use or delete it.
    cardsIn.erase(it);

    Refan();
}

void Hand::Refan() {
    const int N = static_cast<int>(cardsIn.size());
    for (int i = 0; i < N; ++i) {
        float x, y, rotDeg;
        computeLayout(i, N, x, y, rotDeg);
        cardsIn[i].homeX = x;
        cardsIn[i].homeY = y;
        cardsIn[i].homeRotDeg = rotDeg;
        applyHome(cardsIn[i]);
    }
}

void Hand::computeLayout(int i, int N, float& outX, float& outY, float& outRotDeg) const {
    const float R = BASE_R + CENTER_DROP;
    const float Cx = 0.0f, Cy = HAND_Y - R;

    float arcSpacing = W * SEP_FACTOR;
    float stepDeg = (arcSpacing / R) * RAD2DEG;

    float spanDeg = (N > 1) ? stepDeg * float(N - 1) : 0.0f;
    float startDeg = -0.5f * spanDeg;
    float ang = (startDeg + i * stepDeg) * (PI / 180.0f);

    // rim point
    float rimX = Cx + std::sinf(ang) * R;
    float rimY = Cy + std::cosf(ang) * R;

    // normal and tangent
    float nx = std::sinf(ang), ny = std::cosf(ang);
    float tx = ny, ty = -nx;

    // sprite center slightly outside the rim
    outX = rimX + nx * (H * 0.5f - SINK);
    outY = rimY + ny * (H * 0.5f - SINK);
    outRotDeg = std::atan2f(ty, tx) * RAD2DEG;
}

void Hand::applyHome(const CardHand& it)const {
    if (!it.view) return;
    it.view->SetPosition({ it.homeX, it.homeY, 0.0f });
    it.view->SetRotate(it.homeRotDeg);
}

bool Hand::GetHomeFor(GameObject* view, glm::vec3& outPos, float& outRotDeg) const {
    if (!view) return false;
    for (const auto& c : cardsIn) {
        if (c.view == view) {
            outPos = glm::vec3(c.homeX, c.homeY, 0.0f);
            outRotDeg = c.homeRotDeg;
            return true;
        }
    }
    return false;
}
