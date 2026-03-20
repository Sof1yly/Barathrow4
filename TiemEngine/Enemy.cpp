#include "Enemy.h"
#include <iostream>

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

    corruptText = new TextObject();
    corruptText->SetSize(0, 0); // hidden initially

}
void Enemy::setHealth(int h)
{
	health = h;
}

void Enemy::getDamage(int damage)
{
	int total = damage + corruptionStacks;
	health -= total;
	if (health < 0) health = 0;

	if (corruptionStacks > 0)
		std::cout << "[Corrupt] Enemy takes " << corruptionStacks << " extra damage (total " << total << ")" << std::endl;

	SDL_Color white = { 255, 255, 255 };
	hpText->LoadText("HP: " + std::to_string(health), white, 24);
}

void Enemy::addCorruption(int stacks)
{
	corruptionStacks += stacks;
	std::cout << "[Corrupt] Enemy corruption: +" << stacks << " (total " << corruptionStacks << ")" << std::endl;

	if (corruptText) {
		SDL_Color red = { 255, 0, 0 };
		corruptText->LoadText("COR: " + std::to_string(corruptionStacks), red, 20);
	}
}

void Enemy::UpdateTextPosition()
{
    if (!objSprite) return;

    glm::vec3 pos = objSprite->GetPosition();

    if (hpText)
        hpText->SetPosition(glm::vec3(pos.x, pos.y + 80.0f, 100));
}

void Enemy::rotatePattern() {
    patterns[currentPatternIndex] = patterns[currentPatternIndex].rotated90CW();
}
void Enemy::Update(float dt)
{
    if (!objSprite) return;

    glm::vec3 pos = objSprite->GetPosition();

    // HP above enemy
    hpText->SetPosition(glm::vec3(pos.x, pos.y + 80, 200));

    // Corruption text below HP
    if (corruptText)
        corruptText->SetPosition(glm::vec3(pos.x, pos.y + 55, 200));

}

void Enemy::addDelay(int turns)
{
    delayTurns += turns;
    std::cout << "[Delay] Enemy delayed by " << turns << " turn(s). Total delay: " << delayTurns << std::endl;
}

bool Enemy::isDelayed() const
{
    return delayTurns > 0;
}

void Enemy::decrementDelay()
{
    if (delayTurns > 0) {
        delayTurns--;
        std::cout << "[Delay] Enemy delay decremented. Remaining: " << delayTurns << std::endl;
    }
}

Enemy::~Enemy()
{
    if (corruptText) {
        delete corruptText;
        corruptText = nullptr;
    }
    if (hpText) {
        delete hpText;
        hpText = nullptr;
    }
}
