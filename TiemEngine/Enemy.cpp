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

    // TEXT 
    hpText = new TextObject();
    SDL_Color white = { 255,255,255 };
    hpText->LoadText("HP: 10", white, 24);

    corruptText = new TextObject();
    corruptText->SetSize(0, 0);

	//Sprite Create
    objSprite = new SpriteObject("../Resource/Texture/Enemy/Enemy1.png", 4, 12);

    objSprite->SetAnimationLoop(
        0,   // start frame
        0,   // row
        2,   // end frame
        200  // ms per frame
    );
    objSprite->SetSize(200.0f, -200.0f);
    // default position
    setNowPosition(8, 0);
}

void Enemy::setHealth(int h)
{
	health = h;
}

void Enemy::getDamage(int damage)
{
	int total = damage + corruptionStacks;
	health -= total;
    if (health < 0) {
        health = 0;
        isDead = true;
    }
    if (health == 0&& objSprite) {
        objSprite->SetAnimationLoop(
            3,
            0,
            6,
            100
        );

        isTakingDamage = true;
        damageTimer = 0.0f;
    }else
    {
        objSprite->SetAnimationLoop(
            3,
            0, 
            3,
            100 
        );

        isTakingDamage = true;
        damageTimer = 0.0f;
    }

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

	//Reset to idle after damage/attack animation
    if (isTakingDamage)
    {
        damageTimer += dt;

        if (damageTimer >= damageDuration)
        {
            objSprite->SetAnimationLoop(0, 0, 2, 200);

            isTakingDamage = false;
            damageTimer = 0.0f;
        }
    }
    if (isAttacking)
    {
        attackTimer += dt;

        if (attackTimer >= attackDuration)
        {
            objSprite->SetAnimationLoop(0, 0, 2, 200);

            isAttacking = false;
            attackTimer = 0.0f;
        }
    }

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

void Enemy::PlayAttackAnimation(glm::vec3 playerPos)
{
    if (!objSprite) return;

    glm::vec3 enemyPos = objSprite->GetPosition();

    float dx = playerPos.x - enemyPos.x;
    float dy = playerPos.y - enemyPos.y;

    if (abs(dx) > abs(dy))
    {
        if (dx > 0)
        {
			cout << "Enemy attacks right!" << endl;
            objSprite->SetAnimationLoop(2, 7, 6, 200);
        }
        else
        {
			cout << "Enemy attacks left!" << endl;
            objSprite->SetAnimationLoop(2, 0, 6, 200);
        }
    }
    else
    {
        if (dy > 0)
        {
			cout << "Enemy attacks down!" << endl;
            objSprite->SetAnimationLoop(1, 7, 6, 200);
        }
        else
        {
			cout << "Enemy attacks up!" << endl;
            objSprite->SetAnimationLoop(1, 0, 6, 200);
        }
    }
    isAttacking = true;
    attackTimer = 0.0f;
}

Enemy::~Enemy()
{
    if (objSprite) {
        delete objSprite;
        objSprite = nullptr;
    }
    if (corruptText) {
        delete corruptText;
        corruptText = nullptr;
    }
    if (hpText) {
        delete hpText;
        hpText = nullptr;
    }
}
void Enemy::SetWorldPosition(glm::vec3 pos)
{
    if (objSprite)
        objSprite->SetPosition(pos);

    UpdateTextPosition();
}

bool Enemy::isPreparingAttack() const
{
    return preparingAttack;
}

void Enemy::setPreparingAttack(bool value)
{
    preparingAttack = value;
}