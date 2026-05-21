#include "SoundManager.h"
#include <iostream>

// ============================================================
// SOUND FILE TABLE
// One entry per SFX enum value — edit paths here to swap sounds.
// ============================================================
static const char* FILE_PATHS[] =
{
    // ---- Regular enemy attacks ----
    /* ENEMY_A_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_A/NormalA_Attack.mp3",
    /* ENEMY_B_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_B/NormalB_Attack.mp3",
    /* ENEMY_C_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_C/NormalC_Attack.mp3",
    /* ENEMY_D_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_D/EliteA_Attack.mp3",
    /* ENEMY_E_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_E/NormalE_Attack.mp3",
    /* ENEMY_F_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_F/NormalF_Attack.mp3",
    /* ENEMY_G_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_G/NormalG_Attack.mp3",
    /* ENEMY_H_ATTACK  */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_H/NormalG_Attack.mp3",

    // ---- Regular enemy moves ----
    /* ENEMY_A_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_A/NormalA_Move.mp3",
    /* ENEMY_B_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_B/RobotMove.mp3",
    /* ENEMY_C_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_C/RobotMove.mp3",
    /* ENEMY_D_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_D/RobotMove.mp3",
    /* ENEMY_E_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_E/NormalA_Move.mp3",
    /* ENEMY_F_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_F/NormalA_Move.mp3",
    /* ENEMY_G_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_G/RobotMove.mp3",
    /* ENEMY_H_MOVE    */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Type_H/NormalA_Move.mp3",

    // ---- General enemy events ----
    /* ENEMY_TAKE_DAMAGE */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Enemy_TakeDmg.mp3",
    /* ENEMY_DIES        */ "../Resource/Sound/Enemy_Sound/Normal_Enemy/Monster_Dies.mp3",

    // ---- Elite enemies ----
    /* ELITE_A_ATTACK  */ "../Resource/Sound/Enemy_Sound/Elite_Enemy/Elite_A/EliteA_Attack.mp3",
    /* ELITE_B_ATTACK1 */ "../Resource/Sound/Enemy_Sound/Elite_Enemy/Elite_B/Atk_1/NormalF_Attack.mp3",
    /* ELITE_B_ATTACK2 */ "../Resource/Sound/Enemy_Sound/Elite_Enemy/Elite_B/Atk_2/EliteB_Attack2.mp3",
    /* ELITE_C_ATTACK1 */ "../Resource/Sound/Enemy_Sound/Elite_Enemy/Elite_C/Atk_1/NormalG_Attack.mp3",
    /* ELITE_C_ATTACK2 */ "../Resource/Sound/Enemy_Sound/Elite_Enemy/Elite_C/Atk_2/NormalF_Attack.mp3",

    // ---- Boss ----
    /* BOSS_BOARD_WIDE */ "../Resource/Sound/Enemy_Sound/Boss_Enemy/Boss_Stomping.mp3",
    /* BOSS_SUMMON     */ "../Resource/Sound/Enemy_Sound/Boss_Enemy/Boss_Summon.mp3",
    /* BOSS_LASER      */ "../Resource/Sound/Enemy_Sound/Boss_Enemy/NormalA_Attack.mp3",

    // ---- Player ----
    /* PLAYER_ATTACK_MELEE      */ "../Resource/Sound/Player_Sound/Player_Attack/Player_Melee (1).mp3",
    /* PLAYER_ATTACK_MELEE_SPIN */ "../Resource/Sound/Player_Sound/Player_Attack/Player_MeleeSpinNew.mp3",
    /* PLAYER_ATTACK_RANGE      */ "../Resource/Sound/Player_Sound/Player_Attack/Player_Range.mp3",
    /* PLAYER_MOVE              */ "../Resource/Sound/Player_Sound/Player_Move.mp3",
    /* PLAYER_TAKE_DAMAGE       */ "../Resource/Sound/Player_Sound/Player_Hurt.mp3",
    /* PLAYER_DIES              */ "../Resource/Sound/Player_Sound/Player_Dies.mp3",

    // ---- UI ----
    /* UI_CLICK      */ "../Resource/Sound/UI_Sound/UI_Click.mp3",
    /* UI_HOVER_CARD */ "../Resource/Sound/low.wav",
    /* UI_SHOP_BUY   */ "../Resource/Sound/UI_Sound/Shop_Buy.mp3",
    /* UI_LOOT_COIN  */ "../Resource/Sound/UI_Sound/Collecting_Loot.mp3",
};
static_assert(sizeof(FILE_PATHS) / sizeof(FILE_PATHS[0]) == static_cast<int>(SoundManager::SFX::COUNT),
              "FILE_PATHS size must match SoundManager::SFX::COUNT");

// ------------------------------------------------------------
SoundManager& SoundManager::Get()
{
    static SoundManager instance;
    return instance;
}

void SoundManager::Init(AudioEngine& audio)
{
    for (int i = 0; i < SFX_COUNT; i++)
    {
        m_sfx[i] = audio.loadSoundEffect(FILE_PATHS[i]);
    }

    // Load and start BGM — loops forever (-1)
    m_bgm = audio.loadMusic(BGM_FILE);
    m_bgm.play(-1);

    // Apply the volume/mute state already stored in GameData
    // (in case settings were loaded/persisted before Init is called)
    auto* gd = GameData::GetInstance();
    int vol = gd->musicEnabled ? (gd->musicVolume * MIX_MAX_VOLUME / 10) : 0;
    Mix_VolumeMusic(vol);

    m_ready = true;
    std::cout << "[SoundManager] Loaded " << SFX_COUNT << " sound effects. BGM started.\n";
}

void SoundManager::Play(SFX id)
{
    if (!m_ready) return;
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= SFX_COUNT) return;
    m_sfx[idx].play();
}

void SoundManager::PlayEnemyAttack(Enemy::EnemyType type)
{
    int idx = static_cast<int>(SFX::ENEMY_A_ATTACK) + static_cast<int>(type);
    if (idx > static_cast<int>(SFX::ENEMY_H_ATTACK))
        idx = static_cast<int>(SFX::ENEMY_A_ATTACK); // fallback
    Play(static_cast<SFX>(idx));
}

void SoundManager::PlayEnemyMove(Enemy::EnemyType type)
{
    int idx = static_cast<int>(SFX::ENEMY_A_MOVE) + static_cast<int>(type);
    if (idx > static_cast<int>(SFX::ENEMY_H_MOVE))
        idx = static_cast<int>(SFX::ENEMY_A_MOVE); // fallback
    Play(static_cast<SFX>(idx));
}
