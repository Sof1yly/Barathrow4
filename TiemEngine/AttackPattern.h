#pragma once
#include <vector>
#include <string>
#include <utility>

struct IVec2 {
    int x, y;
    IVec2();
    IVec2(int x_, int y_);
    bool operator==(const IVec2& o) const;
};

class AttackPattern {
public:
    using Cell = std::pair<IVec2, int>;

    AttackPattern();

    void addOffset(int dx, int dy, int value = 1);

    static AttackPattern fromGrid(const std::vector<std::string>& grid,
        char markChar = 'X',
        int originCol = -1,
        int originRow = -1,
        int value = 1);

    std::vector<Cell> applyTo(int ox, int oy) const;

    AttackPattern rotated90CW() const;
	AttackPattern rotated90CCW() const;
    AttackPattern mirroredX() const;

    bool hasOffset(int dx, int dy) const;
    size_t size() const;

private:
    std::vector<Cell> cells;
};
