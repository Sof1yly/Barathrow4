#include "Player.h"

int Player::getHp() const { return hp; }
void Player::setHp(int value) { hp = value; }

int Player::getMaxHp() const { return maxHp; }
void Player::setMaxHp(int value) { maxHp = value; }

int Player::getShield() const { return shield; }
int Player::getMaxShield() const { return maxShield; }

void Player::AddShield(int amount)
{
    shield += amount;
    maxShield = shield;
    UpdateShieldBar();

    std::cout << "  Shield: +" << amount
        << " (total: " << shield << ")" << std::endl;
}

void Player::AddBarrier(int amount)
{
 if (amount <= 0) return;

	barrierCount += amount;
   UpdateBarrierTextUI();
	std::cout << "  Barrier: +" << amount << " (total: " << barrierCount << ")" << std::endl;
}

bool Player::ConsumeBarrier()
{
     if (barrierCount <= 0) return false;

    barrierCount--;
	UpdateBarrierTextUI();
	std::cout << "  Barrier negated incoming damage! Remaining: " << barrierCount << std::endl;
	return true;
}

void Player::ExpireBarrier()
{
  if (barrierCount > 0)
	{
		barrierCount--;
		UpdateBarrierTextUI();
		std::cout << "  Barrier reduced by 1 at turn end. Remaining: " << barrierCount << std::endl;
	}
}

bool Player::HasBarrier() const
{
 return barrierCount > 0;
}

// =====================
// Jump Charges
// =====================
void Player::AddJumpCharges(int amount)
{
    if (amount <= 0) return;
    jumpCharges += amount;
    UpdateJumpTextUI();
    std::cout << "  Jump: +" << amount << " charge(s) (total: " << jumpCharges << ")" << std::endl;
}

bool Player::ConsumeJumpCharge()
{
    if (jumpCharges <= 0) return false;
    jumpCharges--;
    UpdateJumpTextUI();
    std::cout << "  Jump charge consumed! Remaining: " << jumpCharges << std::endl;
    return true;
}

void Player::ResetJumpCharges()
{
    if (jumpCharges > 0)
    {
        jumpCharges = 0;
        UpdateJumpTextUI();
        std::cout << "  Jump charges reset at turn end." << std::endl;
    }
}

int Player::GetJumpCharges() const
{
    return jumpCharges;
}

int Player::GetBarrierCount() const
{
	return barrierCount;
}

void Player::ResetShield()
{
    if (shield > 0)
    {
        shield = 0;
        maxShield = 0;
        UpdateShieldBar();
    }
}

int Player::AbsorbDamage(int damage)
{
    if (shield > 0)
    {
        int absorbed = std::min(damage, shield);
        shield -= absorbed;

        UpdateShieldBar();

        std::cout << "    Shield absorbed "
            << absorbed << " damage! Shield = "
            << shield << std::endl;

        return damage - absorbed;
    }
    return damage;
}

void Player::InitShieldUI(std::vector<DrawableObject*>& objectsList)
{
    shieldBg = new ImageObject();
    shieldBg->SetSize(300.0f, -80.0f);
    shieldBg->SetPosition(glm::vec3(-450.0f, 10000.0f, 0.0f));
    shieldBg->SetTexture("../Resource/Texture/UI/Blank_Shieldbar.PNG");
    objectsList.push_back(shieldBg);

    shieldBar = new ImageObject();
    shieldBar->SetSize(300.0f, -80.0f);
    shieldBar->SetPosition(glm::vec3(-450.0f, 10000.0f, 0.0f));
    shieldBar->SetTexture("../Resource/Texture/UI/Shieldbar.PNG");
    objectsList.push_back(shieldBar);

	shieldMask = new ImageObject();
	shieldMask->SetSize(0.0f, -80.0f);
	shieldMask->SetPosition(glm::vec3(-450.0f, 10000.0f, 5.0f));
	shieldMask->SetTexture("../Resource/Texture/UI/HPbarmask.png");
	objectsList.push_back(shieldMask);

	barrierText = new TextObject();
	SDL_Color barrierColor = { 120, 220, 255, 255 };
	barrierText->LoadText("", barrierColor, 22);
	barrierText->SetPosition(glm::vec3(-800.0f, 430.0f, 10.0f));
	objectsList.push_back(barrierText);
	UpdateBarrierTextUI();

	jumpText = new TextObject();
	SDL_Color jumpColor = { 180, 255, 130, 255 };
	jumpText->LoadText("", jumpColor, 22);
	jumpText->SetPosition(glm::vec3(-800.0f, 390.0f, 10.0f));
	objectsList.push_back(jumpText);
	UpdateJumpTextUI();
}

void Player::UpdateBarrierTextUI()
{
	if (!barrierText) return;

	SDL_Color barrierColor = { 120, 220, 255, 255 };
	if (barrierCount > 0)
	{
		barrierText->LoadText("Barrier: " + std::to_string(barrierCount), barrierColor, 22);
		barrierText->SetPosition(glm::vec3(-800.0f, 430.0f, 10.0f));
	}
	else
	{
		barrierText->LoadText("", barrierColor, 22);
	}
}

void Player::UpdateJumpTextUI()
{
	if (!jumpText) return;

	SDL_Color jumpColor = { 180, 255, 130, 255 };
	if (jumpCharges > 0)
	{
		jumpText->LoadText("Jump: " + std::to_string(jumpCharges), jumpColor, 22);
		jumpText->SetPosition(glm::vec3(-800.0f, 390.0f, 10.0f));
	}
	else
	{
		jumpText->LoadText("", jumpColor, 22);
	}
}

void Player::UpdateShieldBar()
{
    if (!shieldBar || !shieldMask || !shieldBg) return;

    if (shield <= 0)
    {
        shieldBg->SetPosition(glm::vec3(-450.0f, 10000.0f, 0.0f));
        shieldBar->SetPosition(glm::vec3(-450.0f, 10000.0f, 0.0f));
        shieldMask->SetPosition(glm::vec3(-450.0f, 10000.0f, 5.0f));
        return;
    }

    shieldBg->SetPosition(glm::vec3(-450.0f, 500.0f, 0.0f));
    shieldBar->SetPosition(glm::vec3(-450.0f, 500.0f, 0.0f));

    float fullWidth = 300.0f;

    shield = std::max(0, std::min(shield, maxShield));
    float percent = (float)shield / (float)maxShield;

    shieldBar->SetSize(fullWidth, -80.0f);

    float missingWidth = fullWidth * (1.0f - percent);
    shieldMask->SetSize(missingWidth, -80.0f);

    float barLeftX = -450.0f;
    float maskX = barLeftX + (fullWidth - missingWidth) / 2.0f;

    shieldMask->SetPosition(glm::vec3(maskX, 500.0f, 5.0f));
}

void Player::InitSprite(std::vector<DrawableObject*>& objectsList, glm::vec3 startPos)
{
    sprite = new SpriteObject("../Resource/Texture/Player_sprite2.png", 9, 16);

    sprite->SetSize(200.0f, -200.0f);
    sprite->SetPosition(startPos);

    sprite->SetAnimationLoop(0, 0, 2, 800);
    sprite->NextAnimation();

    objectsList.push_back(sprite);
}

SpriteObject* Player::GetSprite()
{
    return sprite;
}

void Player::SetPosition(glm::vec3 pos)
{
    if (sprite) sprite->SetPosition(pos);
}

glm::vec3 Player::GetPosition() const
{
    if (sprite) return sprite->GetPosition();
    return glm::vec3(0.0f);
}

void Player::SetPlayerIdle(int dir)
{
    if (!sprite || isDead) return;

    switch (dir)
    {
    case 0: sprite->SetAnimationLoop(0, 0, 2, 800); break;
    case 1: sprite->SetAnimationLoop(0, 2, 2, 800); break;
    case 2: sprite->SetAnimationLoop(0, 4, 2, 800); break;
    case 3: sprite->SetAnimationLoop(0, 6, 2, 800); break;
    }
}

void Player::SetPlayerWalk(int dir)
{
    if (!sprite || isDead) return;

    switch (dir)
    {
    case 0: sprite->SetAnimationLoop(1, 0, 4, 150); break;
    case 1: sprite->SetAnimationLoop(1, 4, 4, 150); break;
    case 2: sprite->SetAnimationLoop(1, 8, 4, 150); break;
    case 3: sprite->SetAnimationLoop(1, 12, 4, 150); break;
    }
}

void Player::SetPlayerAttack(int dir)
{
    if (!sprite || isDead) return;

    switch (dir)
    {
    case 0: sprite->SetAnimationLoop(2, 0, 8, 100); break;
    case 1: sprite->SetAnimationLoop(2, 8, 8, 100); break;
    case 2: sprite->SetAnimationLoop(3, 0, 8, 100); break;
    case 3: sprite->SetAnimationLoop(3, 8, 8, 100); break;
    }
}

void Player::SetPlayerGetDamage(int dir)
{
    if (!sprite || isDead) return;

    switch (dir)
    {
    case 0: sprite->SetAnimationOnce(7, 0, 2, 200); break;
    case 1: sprite->SetAnimationOnce(7, 5, 2, 200); break;
    case 2: sprite->SetAnimationOnce(8, 0, 2, 200); break;
    case 3: sprite->SetAnimationOnce(8, 5, 2, 200); break;
    }
}

void Player::SetPlayerDie(int dir)
{
    if (!sprite) return;
    isDead = true;
    switch (dir)
    {
    case 0: sprite->SetAnimationOnce(7, 0, 5, 200); break;
    case 1: sprite->SetAnimationOnce(7, 5, 5, 200); break;
    case 2: sprite->SetAnimationOnce(8, 0, 5, 200); break;
    case 3: sprite->SetAnimationOnce(8, 5, 5, 200); break;
    }

}