#include "tool.h"
#include "proj.h"

#include <cassert>

void DrawTool::Press(int mapNum, PixPoint const& pos, int b)
{
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }

    if (b & LEFT) {
        Plonk(mapNum, tp, mEd.leftPen);
    }
    if (b & RIGHT) {
        Plonk(mapNum, tp, mEd.rightPen);
    }
}

void DrawTool::Move(int mapNum, PixPoint const& pos, int b)
{
    //printf("Move\n");
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }

    if (b & LEFT) {
        Plonk(mapNum, tp, mEd.leftPen);
    }
    if (b & RIGHT) {
        Plonk(mapNum, tp, mEd.rightPen);
    }
}

void DrawTool::Release(int mapNum, PixPoint const& pos, int b)
{
    //printf("Release\n");
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }
}

void DrawTool::Plonk(int mapNum, TilePoint const& tp, Cell const& pen)
{
    Tilemap& map = mProj.maps[mapNum];
    assert(map.IsValid(tp));
    map.CellAt(tp) = pen;
    mEd.modified = true;
    MapRect dirty(tp, 1, 1);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mapNum, dirty);
    }
}

