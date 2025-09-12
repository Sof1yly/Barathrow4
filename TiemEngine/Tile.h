#pragma once

struct Tile {
    int x;
    int y;
    bool blocked;

    int getX() const { return x; }
    int getY() const { return y; }
    bool isBlocked() const { return blocked; }
};