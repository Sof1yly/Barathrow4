#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class EnemyLoadPattern {
public:
    static void LoadFromFile(const std::string& filename);
    static std::vector<std::string> GetPattern(const std::string& name);

private:
    static std::unordered_map<std::string, std::vector<std::string>> patternMap;
};
