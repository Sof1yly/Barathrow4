#pragma once

#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <random>

using namespace std;

struct Node
{
    int floor;
    int id;

    bool visited = false;

    vector<Node*> next;
};

class LevelManager
{
private:

    vector<vector<Node*>> floors;
    Node* current;

public:

    LevelManager();
    ~LevelManager();

    void Generate();

    void PrintMap();

    void ChoosePath(int choice);
};