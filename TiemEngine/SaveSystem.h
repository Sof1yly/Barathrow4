#pragma once
#include <string>
#include <vector>

struct EnemySaveData {
    int typeIndex        = 0;
    int row              = 0;
    int col              = 0;
    int health           = 0;
    int delayTurns       = 0;
    int corruptionStacks = 0;
    int weakenTurns      = 0;
};

struct SaveData {
    int  playerRow          = 0;
    int  playerCol          = 0;
    int  playerHp           = 50;
    int  playerMaxHp        = 50;
    int  playerCoins        = 100;
    int  playerBarrierCount = 0;
    int  playerJumpCharges  = 0;

    std::vector<EnemySaveData> enemies;
    std::vector<std::string>   cardNames;

    int  currentLevel         = 1;
    int  baseHandSize         = 5;
    bool goldBonusActive      = false;
    int  startCombatBarrier   = 0;
    int  startCombatOverclock = 0;
    bool eventSceneDone       = false;

    // Cards currently in hand when saving (empty = deal a fresh hand on load)
    std::vector<std::string> handCardNames;
};

class SaveSystem {
public:
    static const std::string SAVE_DIR;
    static const std::string SAVE_FILE;

    // Set true before transitioning to a level to trigger save loading in LevelInit.
    static bool pendingLoad;

    static bool Save(const SaveData& data);
    static bool Load(SaveData& outData);
    static bool HasSaveFile();
    static void DeleteSave();

private:
    static bool EnsureSaveDir();
};
