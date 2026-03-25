#pragma once
#include <vector>
#include "DrawableObject.h"
#include "GameObject.h"
#include "AttackPattern.h"
#include <functional>

class HighlightManager
{
public:
    void Init(std::vector<DrawableObject*>& sceneObjects,
        float gridW, float gridH);

    void Clear();

    void ShowAttackPattern(const std::vector<std::pair<IVec2, int>>& cells,
        int gridStartRow, int gridEndRow,
        int gridStartCol, int gridEndCol,
        std::function<glm::vec3(int, int)> gridToWorld);

    void ShowMovePreview(
        int row, int col,
        int steps, int dir,
        int gridStartRow, int gridEndRow,
        int gridStartCol, int gridEndCol,
        std::function<glm::vec3(int, int)> gridToWorld,
        const std::vector<std::pair<int, int>>& enemyPositions
    );

    void ShowEnemyAttack(const std::vector<std::pair<IVec2, int>>& cells,
        int gridStartRow, int gridEndRow,
        int gridStartCol, int gridEndCol,
        std::function<glm::vec3(int, int)> gridToWorld,
        int& index);
    void HideEnemyAttack(int& index);
    void HideAllPlayer();
	void HideAllEnemy();
	void Reset();
private:
    void Hide(std::vector<GameObject*>& list);

    std::vector<GameObject*> attackHighlights;
    std::vector<GameObject*> moveHighlights;
    std::vector<GameObject*> enemyHighlights;

    float gridWide = 0;
    float gridHigh = 0;
};