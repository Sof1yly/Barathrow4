#include "HighlightManager.h"

void HighlightManager::Init(std::vector<DrawableObject*>& sceneObjects,
    float gridW, float gridH)
{
    gridWide = gridW;
    gridHigh = gridH;

    attackHighlights.reserve(20);
    moveHighlights.reserve(10);
    enemyHighlights.reserve(20);

    // Attack highlights (white)
    for (int i = 0; i < 20; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(gridWide, gridHigh);
        h->SetColor(1, 1, 1, 0.4f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        attackHighlights.push_back(h);
        sceneObjects.push_back(h);
    }

    // Move highlight (blue)
    for (int i = 0; i < 10; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(gridWide / 2, gridHigh / 2);
        h->SetColor(0.2f, 0.5f, 1.0f, 0.4f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        moveHighlights.push_back(h);
        sceneObjects.push_back(h);
    }

    // Enemy highlight (red)
    for (int i = 0; i < 20; i++)
    {
        GameObject* h = new GameObject();
        h->SetSize(gridWide, gridHigh);
        h->SetColor(1.0f, 0.2f, 0.2f, 0.45f);
        h->SetPosition(glm::vec3(99999, 99999, 5));
        enemyHighlights.push_back(h);
        sceneObjects.push_back(h);
    }
}
void HighlightManager::Hide(std::vector<GameObject*>& list)
{
    for (auto* h : list)
        h->SetPosition(glm::vec3(99999, 99999, 50));
}
void HighlightManager::HideAllPlayer()
{
    Hide(attackHighlights);
    Hide(moveHighlights);
}
void HighlightManager::HideAllEnemy()
{
    Hide(enemyHighlights);
}
void HighlightManager::ShowAttackPattern(
    const std::vector<std::pair<IVec2, int>>& cells,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    std::function<glm::vec3(int, int)> gridToWorld)
{
    Hide(attackHighlights);

    int index = 0;
    for (auto& cell : cells)
    {
        int gx = cell.first.x;
        int gy = cell.first.y;

        if (gx < gridStartRow || gx >= gridEndRow ||
            gy < gridStartCol || gy >= gridEndCol)
            continue;

        if (index >= attackHighlights.size())
            break;

        glm::vec3 world = gridToWorld(gx, gy);
        attackHighlights[index]->SetPosition(
            glm::vec3(world.x, world.y, 30)
        );
        index++;
    }
}

void HighlightManager::ShowMovePreview(
    int row, int col,
    int steps, int dir,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    std::function<glm::vec3(int, int)> gridToWorld,
    const std::vector<std::pair<int, int>>& enemyPositions)
{
    Hide(moveHighlights);

    int r = row;
    int c = col;

    std::vector<std::pair<int, int>> validTiles;

    for (int i = 0; i < steps; i++)
    {
        int nextR = r;
        int nextC = c;

        switch (dir)
        {
        case 0: nextR--; break; // up
        case 1: nextC--; break; // left
        case 2: nextC++; break; // right
        case 3: nextR++; break; // down
        }

        if (nextR < gridStartRow || nextR >= gridEndRow ||
            nextC < gridStartCol || nextC >= gridEndCol)
            break;

        bool blocked = false;
        for (auto& pos : enemyPositions)
        {
            if (nextR == pos.first && nextC == pos.second)
            {
                blocked = true;
                break;
            }
        }
        if (blocked) break;

        r = nextR;
        c = nextC;

        validTiles.emplace_back(r, c);
    }

    for (int i = 0; i < validTiles.size() && i < moveHighlights.size(); i++)
    {
        int tileR = validTiles[i].first;
        int tileC = validTiles[i].second;

        glm::vec3 world = gridToWorld(tileR, tileC);
        moveHighlights[i]->SetPosition(glm::vec3(world.x, world.y, 60));

        if (i == validTiles.size() - 1)
            moveHighlights[i]->SetColor(0.2f, 1.0f, 0.2f, 0.6f);
        else
            moveHighlights[i]->SetColor(0.2f, 0.5f, 1.0f, 0.4f);
    }
}

void HighlightManager::ShowEnemyAttack(
    const std::vector<std::pair<IVec2, int>>& cells,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    std::function<glm::vec3(int, int)> gridToWorld)
{
    Hide(enemyHighlights);

    int index = 0;
    for (auto& cell : cells)
    {
        int gx = cell.first.x;
        int gy = cell.first.y;

        if (gx < gridStartRow || gx >= gridEndRow ||
            gy < gridStartCol || gy >= gridEndCol)
            continue;

        if (index >= enemyHighlights.size())
            break;

        glm::vec3 world = gridToWorld(gx, gy);
        enemyHighlights[index]->SetPosition(
            glm::vec3(world.x, world.y, 40)
        );

        index++;
    }
}

void HighlightManager::Reset()
{
    attackHighlights.clear();
    moveHighlights.clear();
    enemyHighlights.clear();
}