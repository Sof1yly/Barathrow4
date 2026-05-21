#pragma once
#include "Audio.h"
#include "Enemy.h"

// ============================================================
// SoundManager — central registry for all in-game sound effects
//
// HOW TO CHANGE A SOUND:
//   Open SoundManager.cpp and edit FILE_PATHS[].
//   Each index matches the SFX enum value below.
// ============================================================

class SoundManager
{
public:
    enum class SFX
    {
        // ---- Regular enemy attacks (index = Enemy::EnemyType cast) ----
        ENEMY_A_ATTACK = 0,
        ENEMY_B_ATTACK,
        ENEMY_C_ATTACK,
        ENEMY_D_ATTACK,
        ENEMY_E_ATTACK,
        ENEMY_F_ATTACK,
        ENEMY_G_ATTACK,
        ENEMY_H_ATTACK,

        // ---- Regular enemy moves ----
        ENEMY_A_MOVE,
        ENEMY_B_MOVE,
        ENEMY_C_MOVE,
        ENEMY_D_MOVE,
        ENEMY_E_MOVE,
        ENEMY_F_MOVE,
        ENEMY_G_MOVE,
        ENEMY_H_MOVE,

        // ---- General enemy events ----
        ENEMY_TAKE_DAMAGE,
        ENEMY_DIES,

        // ---- Elite enemies ----
        ELITE_A_ATTACK,
        ELITE_B_ATTACK1,
        ELITE_B_ATTACK2,
        ELITE_C_ATTACK1,
        ELITE_C_ATTACK2,

        // ---- Boss ----
        BOSS_BOARD_WIDE,   // patterns 1 / 2 / 3  (grid-wide sweep)
        BOSS_SUMMON,       // pattern 9            (summon minions)
        BOSS_LASER,        // patterns 6 / 7 (falling) + 8 (cross laser)

        COUNT   // keep last
    };

    static SoundManager& Get();

    // Call once in Main.cpp after audio.init()
    void Init(AudioEngine& audio);

    // Play a sound by explicit ID
    void Play(SFX id);

    // Convenience helpers used by Level.cpp
    void PlayEnemyAttack(Enemy::EnemyType type);
    void PlayEnemyMove  (Enemy::EnemyType type);

private:
    SoundManager() = default;

    static constexpr int SFX_COUNT = static_cast<int>(SFX::COUNT);
    SoundEffect m_sfx[SFX_COUNT];
    bool        m_ready = false;
};
