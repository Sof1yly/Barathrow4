#pragma once

struct Rect { float xmin, ymin, xmax, ymax; };

enum class DropZone { None, Left, Right, Top, Bottom };

struct DropZones {
    Rect left{}, right{}, top{}, bottom{};

    // Call whenever resolution/viewport might change.
    void UpdateFromViewport(float drawW, float drawH) {
        const float yHandTop = -drawH * 0.5f + drawH * 0.28f;   // hand boundary
        const float yPlayTop = drawH * 0.5f - drawH * 0.08f;   // top margin
        const float yPlayBot = yHandTop;

        const float xLeftEdge = -drawW * 0.5f + drawW * 0.06f;   // left margin
        const float xRightEdge = drawW * 0.5f - drawW * 0.06f;   // right margin

        const float sideZoneW = drawW * 0.15f;
        left = { xLeftEdge, yPlayBot, xLeftEdge + sideZoneW, yPlayTop };
        right = { xRightEdge - sideZoneW, yPlayBot, xRightEdge, yPlayTop };

        const float midX0 = xLeftEdge + sideZoneW + drawW * 0.02f;
        const float midX1 = xRightEdge - sideZoneW - drawW * 0.02f;

        const float midBandH = (yPlayTop - yPlayBot);
        const float topZoneH = midBandH * 0.22f;
        const float botZoneH = midBandH * 0.22f;
        const float dividerH = midBandH * 0.02f;
        const float midY = (yPlayBot + yPlayTop) * 0.5f;

        top = { midX0, midY + dividerH * 0.5f, midX1, midY + dividerH * 0.5f + topZoneH };
        bottom = { midX0, midY - dividerH * 0.5f - botZoneH, midX1, midY - dividerH * 0.5f };
    }

    static bool PointIn(const Rect& r, float x, float y) {
        return (x >= r.xmin && x <= r.xmax && y >= r.ymin && y <= r.ymax);
    }

    DropZone Classify(float x, float y) const {
        if (PointIn(left, x, y)) return DropZone::Left;
        if (PointIn(right, x, y)) return DropZone::Right;
        if (PointIn(top, x, y)) return DropZone::Top;
        if (PointIn(bottom, x, y)) return DropZone::Bottom;
        return DropZone::None;
    }
};
