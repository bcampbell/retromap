#include "cmd.h"
#include "tool.h"
#include "proj.h"
#include "mapview.h"
#include "editor.h"

#include <cassert>

Tool::Tool(Editor& ed) : mEd(ed), mProj(ed.proj)
{
}

Tool::~Tool()
{
}

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

    // TODO: move this out into view
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


void PickupTool::Press(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }

    mLatch = b;
    mAnchor = tp;

    view->SetCursor(MapRect(mAnchor, 1, 1));
}

void PickupTool::Move(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    if (!mLatch) {
        return;
    }
    TilePoint tp = mProj.ToTilePoint(pos);
    Tilemap& map = mProj.maps[mapNum];
    if (!map.IsValid(tp)) {
        return;
    }

    MapRect c;
    int xmin = std::min(tp.x, mAnchor.x);
    int ymin = std::min(tp.y, mAnchor.y);
    int xmax = std::max(tp.x, mAnchor.x);
    int ymax = std::max(tp.y, mAnchor.y);
    view->SetCursor(MapRect(TilePoint(xmin,ymin), 1+xmax-xmin, 1+ymax-ymin));
}

void PickupTool::Release(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    if (mLatch) {
        mLatch = 0;
        view->HideCursor();
        mEd.SetTool(TOOL_DRAW);
    }
}

void PickupTool::Reset()
{
    mLatch = 0;
}

