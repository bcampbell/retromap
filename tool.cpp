#include "cmd.h"
#include "tool.h"
#include "proj.h"
#include "mapview.h"

#include <cassert>

void DrawTool::Press(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    TilePoint tp = mProj.ToTilePoint(pos);
    mPrevPos = tp;
    Tilemap& map = mProj.maps[mapNum];

    if (!mCmd) {
        mCmd = new MapDrawCmd(mEd, mapNum);
    }


    if (!map.IsValid(tp)) {
        return;
    }

    if (b & LEFT) {
        mCmd->Plonk(tp, mEd.leftPen);
    }
    if (b & RIGHT) {
        mCmd->Plonk(tp, mEd.rightPen);
    }
}

void DrawTool::Move(MapView* view, int mapNum, PixPoint const& pos, int b)
{

    //printf("Move\n");
    TilePoint tp = mProj.ToTilePoint(pos);

    view->SetCursor(MapRect(tp,1,1));

    if (!mCmd) {
        return;
    }

    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }

    if (tp == mPrevPos) {
        return;
    }
    mPrevPos = tp;

    if (b & LEFT) {
        mCmd->Plonk(tp, mEd.leftPen);
    }
    if (b & RIGHT) {
        mCmd->Plonk(tp, mEd.rightPen);
    }
}

void DrawTool::Release(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    //printf("Release\n");
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];

    if (mCmd) {
        mCmd->Commit(); // no more drawing.
        mEd.AddCmd(mCmd);
        mCmd = nullptr;
    }

    if (!map.IsValid(tp)) {
        return;
    }
}

