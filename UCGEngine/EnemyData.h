#pragma once
#include <string>
#include <unordered_map>

struct EnemyData {
    int hp = 0;
    int atk = 0;
    int mov = 0;
    int countdown = 0;
    std::string pattern;
    int attackIncrement = 0;
};

class EnemyDatabase {
public:
    static void LoadFromFile(const std::string& filename);
    static EnemyData GetData(const std::string& type);

private:
    static std::unordered_map<std::string, EnemyData> dataMap;
};