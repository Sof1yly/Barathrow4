#include "AttackPattern.h"
#include <iostream>


IVec2::IVec2() : x(0), y(0) {}
IVec2::IVec2(int x_, int y_) : x(x_), y(y_) {}
bool IVec2::operator==(const IVec2& o) const { return x == o.x && y == o.y; }

AttackPattern::AttackPattern() = default;

void AttackPattern::addOffset(int dx, int dy, int value) {
    cells.emplace_back(IVec2(dx, dy), value);
}

AttackPattern AttackPattern::fromGrid(const std::vector<std::string>& grid,
    char markChar,
    int originCol,
    int originRow,
    int value)
{
    AttackPattern p;
    int rows = static_cast<int>(grid.size());
    if (rows == 0) return p;
    int cols = static_cast<int>(grid[0].size());

    if (originCol < 0) originCol = cols / 2;
    if (originRow < 0) originRow = rows / 2;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < static_cast<int>(grid[r].size()); ++c) {
            if (grid[r][c] == markChar) {
                int dx = c - originCol;
                int dy = r - originRow; // y positive downward
                p.addOffset(dx, dy, value);
            }
        }
    }
    return p;
}

std::vector<AttackPattern::Cell> AttackPattern::applyTo(int ox, int oy) const {
    std::vector<Cell> out;
    out.reserve(cells.size());
    for (auto& c : cells)
        out.emplace_back(IVec2(ox + c.first.x, oy + c.first.y), c.second);
    return out;
}

AttackPattern AttackPattern::rotated90CW() const {
    AttackPattern p;
    for (auto& c : cells) {
        int nx = c.first.y;
        int ny = -c.first.x;
        p.addOffset(nx, ny, c.second);
    }
    return p;
}

AttackPattern AttackPattern::mirroredX() const {
    AttackPattern p;
    for (auto& c : cells)
        p.addOffset(-c.first.x, c.first.y, c.second);
    return p;
}

bool AttackPattern::hasOffset(int dx, int dy) const {
    for (auto& c : cells)
        if (c.first.x == dx && c.first.y == dy)
            return true;
    return false;
}

size_t AttackPattern::size() const {
    return cells.size();
}
