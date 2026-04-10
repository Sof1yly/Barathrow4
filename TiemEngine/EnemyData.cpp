#include "EnemyData.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::unordered_map<std::string, EnemyData> EnemyDatabase::dataMap;

void EnemyDatabase::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open EnemyData file\n";
        return;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);

        std::string type;
        EnemyData d;

        ss >> type >> d.hp >> d.atk >> d.mov >> d.countdown >> d.pattern >> d.attackIncrement;

        dataMap[type] = d;
    }

    file.close();
}

EnemyData EnemyDatabase::GetData(const std::string& type)
{
    if (dataMap.find(type) != dataMap.end())
        return dataMap[type];

    std::cout << "Enemy type not found: " << type << std::endl;
    return EnemyData();
}