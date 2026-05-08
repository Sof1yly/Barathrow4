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
    const bool walkable[][GRID_COLS],
    std::function<glm::vec3(int, int)> gridToWorld)
{
    Hide(attackHighlights);

    int index = 0;

    for (const auto& item : cells)
    {
        if (index >= attackHighlights.size())
            break;

        int r = item.first.x;
        int c = item.first.y;

        if (r < gridStartRow || r >= gridEndRow ||
            c < gridStartCol || c >= gridEndCol)
            continue;

        if (!walkable[r][c])
            continue;

        GameObject* obj = attackHighlights[index];

        obj->SetColor(1.0f, 1.0f, 1.0f,0.5f);
        obj->SetAlpha(0.45f);

        obj->SetSize(gridWide - 10.0f, gridHigh - 10.0f);

        glm::vec3 pos = gridToWorld(r, c);
        pos.z = 5.0f;

        obj->SetPosition(pos);

        index++;
    }
}
void HighlightManager::ShowMovePreview(
    int row, int col,
    int steps, int dir,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    const bool walkable[][GRID_COLS],
    std::function<glm::vec3(int, int)> gridToWorld,
    const std::vector<std::pair<int, int>>& enemyPositions)
{
    Hide(moveHighlights);

    int r = row;
    int c = col;

    int index = 0;

    for (int i = 0; i < steps; i++)
    {
        if (index >= moveHighlights.size())
            break;

        switch (dir)
        {
        case 0: r--; break;
        case 1: c--; break;
        case 2: c++; break;
        case 3: r++; break;
        }

        if (r < gridStartRow || r >= gridEndRow ||
            c < gridStartCol || c >= gridEndCol)
            break;

        if (!walkable[r][c])
            break;

        bool blocked = false;

        for (const auto& e : enemyPositions)
        {
            if (e.first == r && e.second == c)
            {
                blocked = true;
                break;
            }
        }

        if (blocked)
            break;

        GameObject* obj = moveHighlights[index];

        obj->SetColor(0.0f, 1.0f, 0.0f,0.3f);
        obj->SetAlpha(0.45f);

        obj->SetSize(gridWide - 15.0f -30.0f, gridHigh - 15.0f-30.0f);

        glm::vec3 pos = gridToWorld(r, c);
        pos.z = 4.0f;

        obj->SetPosition(pos);

        index++;
    }
}

void HighlightManager::ShowEnemyAttack(
    const std::vector<std::pair<IVec2, int>>& cells,
    int gridStartRow, int gridEndRow,
    int gridStartCol, int gridEndCol,
    const bool walkable[][GRID_COLS],
    std::function<glm::vec3(int, int)> gridToWorld,
    int& index)
{
    for (const auto& item : cells)
    {
        if (index >= enemyHighlights.size())
            break;

        int r = item.first.x;
        int c = item.first.y;

        if (r < gridStartRow || r >= gridEndRow ||
            c < gridStartCol || c >= gridEndCol)
            continue;

        if (!walkable[r][c])
            continue;

        GameObject* obj = enemyHighlights[index];

        obj->SetColor(1.0f, 0.0f, 0.0f,0.5f);
        obj->SetAlpha(0.5f);

        obj->SetSize(gridWide - 12.0f, gridHigh - 12.0f);

        glm::vec3 pos = gridToWorld(r, c);
        pos.z = 3.0f;

        obj->SetPosition(pos);

        index++;
    }
}
void HighlightManager::HideEnemyAttack(int& index)
{
    if (index <= 0) return;

    for (int i = 0; i < index && i < enemyHighlights.size(); i++)
    {
        enemyHighlights[i]->SetPosition(glm::vec3(99999, 99999, 50));
    }

    index = 0;
}

void HighlightManager::Reset()
{
    attackHighlights.clear();
    moveHighlights.clear();
    enemyHighlights.clear();
}