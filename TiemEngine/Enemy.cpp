#include "Enemy.h"

Enemy::Enemy()
{
	health = maxHealth;
    patterns = {
            AttackPattern::fromGrid({
                ".X.",
                "XXX",
                ".X."
            }, 'X'),

            AttackPattern::fromGrid({
                "XXX",
                "XXX",
                "XXX"
            }, 'X'),

            AttackPattern::fromGrid({
                "..X..",
                ".XXX.",
                "XXXXX",
                ".XXX.",
                "..X.."
            }, 'X'),

            AttackPattern::fromGrid({
                "...XX",
            }, 'X'),
    };
    hpText = new TextObject();
    SDL_Color white = { 255,255,255 };
    hpText->LoadText("HP: 10", white, 24);

}
void Enemy::setHealth(int h)
{
	health = h;
}

void Enemy::getDamage(int damage)
{
	health -= damage;
    if (health < 0) health = 0;


    SDL_Color white = { 255, 255, 255 };
    hpText->LoadText("HP: " + std::to_string(health), white, 24);
}

void Enemy::UpdateTextPosition()
{
    if (!objImg) return;

    glm::vec3 pos = objImg->GetPosition();

    if (hpText)
        hpText->SetPosition(glm::vec3(pos.x, pos.y + 80.0f, 100));
}

void Enemy::rotatePattern() {
    patterns[currentPatternIndex] = patterns[currentPatternIndex].rotated90CW();
}
void Enemy::Update(float dt)
{
    if (!objImg) return;

    glm::vec3 pos = objImg->GetPosition();

    // HP above enemy
    hpText->SetPosition(glm::vec3(pos.x, pos.y + 80, 200));

}

Enemy::~Enemy()
{
    if (hpText) {
        delete hpText;
        hpText = nullptr;
    }
}
