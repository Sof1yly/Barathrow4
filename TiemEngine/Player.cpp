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
	std::cout << "  Shield: +" << amount << " (total: " << shield << ")" << std::endl;
}

void Player::AddBarrier()
{
	barrierActive = true;
	std::cout << "  Barrier: active" << std::endl;
}

bool Player::ConsumeBarrier()
{
	if (!barrierActive) return false;

	barrierActive = false;
	std::cout << "  Barrier negated incoming damage!" << std::endl;
	return true;
}

void Player::ExpireBarrier()
{
	if (barrierActive)
	{
		barrierActive = false;
		std::cout << "  Barrier expired at turn end." << std::endl;
	}
}

bool Player::HasBarrier() const
{
	return barrierActive;
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
		std::cout << "    Shield absorbed " << absorbed << " damage! Shield = " << shield << std::endl;
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
