#include "SoundManager.h"
#include <iostream>

// ============================================================
// SOUND FILE TABLE
// One entry per SFX enum value — edit paths here to swap sounds.
// ============================================================
static const char* FILE_PATHS[] =
{
    // ---- Regular enemy attacks ----
    /* ENEMY_A_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_B_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_C_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_D_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_E_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_F_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_G_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ENEMY_H_ATTACK  */ "../Resource/Sound/medium.wav",

    // ---- Regular enemy moves ----
    /* ENEMY_A_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_B_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_C_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_D_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_E_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_F_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_G_MOVE    */ "../Resource/Sound/low.wav",
    /* ENEMY_H_MOVE    */ "../Resource/Sound/low.wav",

    // ---- General enemy events ----
    /* ENEMY_TAKE_DAMAGE */ "../Resource/Sound/scratch.wav",
    /* ENEMY_DIES        */ "../Resource/Sound/high.wav",

    // ---- Elite enemies ----
    /* ELITE_A_ATTACK  */ "../Resource/Sound/medium.wav",
    /* ELITE_B_ATTACK1 */ "../Resource/Sound/medium.wav",
    /* ELITE_B_ATTACK2 */ "../Resource/Sound/scratch.wav",
    /* ELITE_C_ATTACK1 */ "../Resource/Sound/medium.wav",
    /* ELITE_C_ATTACK2 */ "../Resource/Sound/scratch.wav",
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
    m_ready = true;
    std::cout << "[SoundManager] Loaded " << SFX_COUNT << " sound effects.\n";
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
