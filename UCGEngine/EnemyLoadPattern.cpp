#include "EnemyLoadPattern.h"
#include <fstream>
#include <iostream>

std::unordered_map<std::string, std::vector<std::string>> EnemyLoadPattern::patternMap;

void EnemyLoadPattern::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open Pattern file\n";
        return;
    }

    std::string line;
    std::string currentPattern = "";

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        if (line[0] == 'A') // new pattern
        {
            currentPattern = line;
            patternMap[currentPattern] = {};
        }
        else if (!currentPattern.empty())
        {
            patternMap[currentPattern].push_back(line);
        }
    }

    file.close();
}

std::vector<std::string> EnemyLoadPattern::GetPattern(const std::string& name)
{
    if (patternMap.find(name) != patternMap.end())
        return patternMap[name];

    std::cout << "Pattern not found: " << name << std::endl;
    return {};
}