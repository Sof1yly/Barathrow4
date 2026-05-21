#include "SaveSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>     // std::remove
#include <direct.h>   // _mkdir on Windows

const std::string SaveSystem::SAVE_DIR      = "../Resource/GameData/Save";
const std::string SaveSystem::SAVE_FILE     = "../Resource/GameData/Save/save.dat";
const std::string SaveSystem::PLAYED_FILE   = "../Resource/GameData/Save/played.dat";
const std::string SaveSystem::SETTINGS_FILE = "../Resource/GameData/Save/settings.dat";
bool              SaveSystem::pendingLoad   = false;

bool SaveSystem::EnsureSaveDir()
{
    _mkdir(SAVE_DIR.c_str());
    return true;
}

bool SaveSystem::Save(const SaveData& d)
{
    EnsureSaveDir();
    std::ofstream f(SAVE_FILE);
    if (!f.is_open()) {
        std::cerr << "[Save] Cannot open " << SAVE_FILE << "\n";
        return false;
    }

    f << "PLAYER_ROW="    << d.playerRow          << "\n";
    f << "PLAYER_COL="    << d.playerCol          << "\n";
    f << "PLAYER_HP="     << d.playerHp           << "\n";
    f << "PLAYER_MAX_HP=" << d.playerMaxHp        << "\n";
    f << "PLAYER_COINS="  << d.playerCoins        << "\n";
    f << "PLAYER_BARRIER="<< d.playerBarrierCount << "\n";
    f << "PLAYER_JUMPS="  << d.playerJumpCharges  << "\n";
    f << "LEVEL="         << d.currentLevel       << "\n";
    f << "BASE_HAND="     << d.baseHandSize        << "\n";
    f << "GOLD_BONUS="    << (d.goldBonusActive ? 1 : 0) << "\n";
    f << "START_BARRIER=" << d.startCombatBarrier  << "\n";
    f << "START_OC="      << d.startCombatOverclock << "\n";
    f << "EVENT_DONE="    << (d.eventSceneDone ? 1 : 0) << "\n";

    f << "ENEMY_COUNT=" << d.enemies.size() << "\n";
    for (const auto& e : d.enemies)
        f << "ENEMY " << e.typeIndex << " " << e.row << " " << e.col << " "
          << e.health << " " << e.delayTurns << " " << e.corruptionStacks << " "
          << e.weakenTurns << " " << e.stunTurns << "\n";

    f << "CARD_COUNT=" << d.cardNames.size() << "\n";
    for (const auto& name : d.cardNames)
        f << "CARD " << name << "\n";

    f << "HAND_COUNT=" << d.handCardNames.size() << "\n";
    for (const auto& name : d.handCardNames)
        f << "HAND " << name << "\n";

    std::cout << "[Save] Saved to " << SAVE_FILE << "\n";
    return true;
}

bool SaveSystem::Load(SaveData& d)
{
    std::ifstream f(SAVE_FILE);
    if (!f.is_open()) {
        std::cerr << "[Save] No save file at " << SAVE_FILE << "\n";
        return false;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;

        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            if      (key == "PLAYER_ROW")    d.playerRow          = std::stoi(val);
            else if (key == "PLAYER_COL")    d.playerCol          = std::stoi(val);
            else if (key == "PLAYER_HP")     d.playerHp           = std::stoi(val);
            else if (key == "PLAYER_MAX_HP") d.playerMaxHp        = std::stoi(val);
            else if (key == "PLAYER_COINS")  d.playerCoins        = std::stoi(val);
            else if (key == "PLAYER_BARRIER")d.playerBarrierCount = std::stoi(val);
            else if (key == "PLAYER_JUMPS")  d.playerJumpCharges  = std::stoi(val);
            else if (key == "LEVEL")         d.currentLevel       = std::stoi(val);
            else if (key == "BASE_HAND")     d.baseHandSize       = std::stoi(val);
            else if (key == "GOLD_BONUS")    d.goldBonusActive    = std::stoi(val) != 0;
            else if (key == "START_BARRIER") d.startCombatBarrier = std::stoi(val);
            else if (key == "START_OC")      d.startCombatOverclock = std::stoi(val);
            else if (key == "EVENT_DONE")    d.eventSceneDone     = std::stoi(val) != 0;
            continue;
        }

        // Space-delimited records
        if (line.substr(0, 6) == "ENEMY ") {
            std::istringstream ss(line.substr(6));
            EnemySaveData e;
            ss >> e.typeIndex >> e.row >> e.col >> e.health
               >> e.delayTurns >> e.corruptionStacks >> e.weakenTurns >> e.stunTurns;
            d.enemies.push_back(e);
        } else if (line.substr(0, 5) == "CARD ") {
            d.cardNames.push_back(line.substr(5));
        } else if (line.substr(0, 5) == "HAND ") {
            d.handCardNames.push_back(line.substr(5));
        }
    }

    std::cout << "[Save] Loaded from " << SAVE_FILE << "\n";
    return true;
}

bool SaveSystem::HasSaveFile()
{
    std::ifstream f(SAVE_FILE);
    return f.good();
}

void SaveSystem::DeleteSave()
{
    std::remove(SAVE_FILE.c_str());
    std::cout << "[Save] Save file deleted.\n";
}

bool SaveSystem::HasPlayedBefore()
{
    std::ifstream f(PLAYED_FILE);
    return f.good();
}

void SaveSystem::MarkPlayed()
{
    EnsureSaveDir();
    std::ofstream f(PLAYED_FILE);
    if (f.is_open())
    {
        f << "played\n";
        std::cout << "[Save] Marked as played.\n";
    }
}

bool SaveSystem::SaveSettings(const SettingsData& s)
{
    EnsureSaveDir();
    std::ofstream f(SETTINGS_FILE);
    if (!f.is_open()) return false;

    f << "MUSIC_VOL="     << s.musicVolume     << "\n";
    f << "MUSIC_ON="      << (s.musicEnabled ? 1 : 0) << "\n";
    f << "SOUND_VOL="     << s.soundVolume     << "\n";
    f << "SOUND_ON="      << (s.soundEnabled ? 1 : 0) << "\n";
    f << "RESOLUTION="    << s.resolutionIndex << "\n";
    return true;
}

bool SaveSystem::LoadSettings(SettingsData& s)
{
    std::ifstream f(SETTINGS_FILE);
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line))
    {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if      (key == "MUSIC_VOL")  s.musicVolume     = std::stoi(val);
        else if (key == "MUSIC_ON")   s.musicEnabled    = std::stoi(val) != 0;
        else if (key == "SOUND_VOL")  s.soundVolume     = std::stoi(val);
        else if (key == "SOUND_ON")   s.soundEnabled    = std::stoi(val) != 0;
        else if (key == "RESOLUTION") s.resolutionIndex = std::stoi(val);
    }
    return true;
}
