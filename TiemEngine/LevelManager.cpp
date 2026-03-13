#include "LevelManager.h"
#include <algorithm>

LevelManager::LevelManager()
{
    srand((unsigned)time(0));
    Generate();
}

LevelManager::~LevelManager()
{
    for (auto& floor : floors)
    {
        for (auto node : floor)
        {
            delete node;
        }
    }
}

void LevelManager::Generate()
{
    floors.clear();
    floors.resize(11);

    random_device rd;
    default_random_engine rng(rd());

    // -------- Floor 1 --------
    Node* start = new Node{ 1,0,false };
    floors[1].push_back(start);
    current = start;

    // -------- Floor 2 - 8 --------
    for (int f = 2; f <= 8; f++)
    {
        int count = rand() % 3 + 1; // 1-3 nodes only

        for (int i = 0; i < count; i++)
        {
            floors[f].push_back(new Node{ f,i,false });
        }
    }

    // -------- Floor 9 --------
    Node* f9 = new Node{ 9,0,false };
    floors[9].push_back(f9);

    // -------- Floor 10 --------
    Node* boss = new Node{ 10,0,false };
    floors[10].push_back(boss);

    // CONNECT FLOORS 1 -> 8

    for (int f = 1; f <= 7; f++)
    {
        auto& prevFloor = floors[f];
        auto& nextFloor = floors[f + 1];

        int nextCount = nextFloor.size();

        // ensure every node in next floor has at least one incoming link
        for (int i = 0; i < nextCount; i++)
        {
            Node* prev = prevFloor[rand() % prevFloor.size()];
            prev->next.push_back(nextFloor[i]);
        }

        // ensure every node has at least one outgoing link
        for (auto prev : prevFloor)
        {
            if (prev->next.empty())
            {
                Node* target = nextFloor[rand() % nextCount];
                prev->next.push_back(target);
            }
        }

        // optional extra random links
        for (auto prev : prevFloor)
        {
            int extra = rand() % 2; // 0-1 extra
            for (int e = 0; e < extra; e++)
            {
                Node* target = nextFloor[rand() % nextCount];

                if (find(prev->next.begin(), prev->next.end(), target) == prev->next.end())
                    prev->next.push_back(target);
            }
        }
    }

    // FLOOR 8 -> FLOOR 9 

    for (auto n : floors[8])
    {
        n->next.push_back(f9);
    }

    // FLOOR 9 -> FLOOR 10


    f9->next.push_back(boss);
}
void LevelManager::PrintMap()
{
    const int WIDTH = 20;
    const int HEIGHT = 25;

    vector<string> grid(HEIGHT, string(WIDTH, ' '));

    int startY = HEIGHT - 2;
    int floorSpacing = 2;

    map<Node*, pair<int, int>> pos;

    for (int f = 1; f <= 10; f++)
    {
        int count = (int)floors[f].size();

        int y = startY - (f - 1) * floorSpacing;
        int spacing = WIDTH / (count + 1);

        for (int i = 0; i < count; i++)
        {
            int x = spacing * (i + 1);

            Node* n = floors[f][i];

            char c = (n == current) ? '@' : (n->visited ? '*' : 'x');

            if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH)
                grid[y][x] = c;

            pos[n] = { x,y };
        }
    }
    // draw simple connections
    for (int f = 1; f <= 9; f++)
    {
        for (auto n : floors[f])
        {
            auto p1 = pos[n];
            int x1 = p1.first;
            int y1 = p1.second;

            for (auto next : n->next)
            {
                auto p2 = pos[next];
                int x2 = p2.first;
                int y2 = p2.second;

                int x = x1;
                int y = y1 - 1;

                while (y > y2)
                {
                    if (x < x2)
                    {
                        x++;
                        grid[y][x] = '/';
                    }
                    else if (x > x2)
                    {
                        x--;
                        grid[y][x] = '\\';
                    }
                    else
                    {
                        grid[y][x] = '|';
                    }

                    y--;
                }
            }
        }
    }

    cout << "\n======= MAP =======\n\n";

    for (int y = 0; y < HEIGHT; y++)
        cout << grid[y] << endl;

    cout << "\nAvailable paths: ";

    for (int i = 0; i < (int)current->next.size(); i++)
        cout << i + 1 << " ";

    cout << endl;
}

void LevelManager::ChoosePath(int choice)
{
    if (choice <= 0 || choice > (int)current->next.size())
        return;

    current->visited = true;
    current = current->next[choice - 1];

    cout << "Moved to Floor " << current->floor << endl;
}