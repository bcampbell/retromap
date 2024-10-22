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
        if (mEd.useBrush) {
            mCmd->DrawBrush(tp, mEd.brush, mEd.rightPen);
        } else {
            mCmd->Plonk(tp, mEd.leftPen);
        }
    }
    if (b & RIGHT) {
        if (mEd.useBrush) {
            mCmd->EraseBrush(tp, mEd.brush, mEd.rightPen);
        } else {
            mCmd->Plonk(tp, mEd.rightPen);
        }
    }
}

void DrawTool::Move(MapView* view, int mapNum, PixPoint const& pos, int b)
{

    //printf("Move\n");
    TilePoint tp = mProj.ToTilePoint(pos);

    if (mEd.useBrush) {
        view->SetCursor(MapRect(tp, mEd.brush.w, mEd.brush.h));
    } else {
        view->SetCursor(MapRect(tp,1,1));
    }

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
        if (mEd.useBrush) {
            mCmd->DrawBrush(tp, mEd.brush, mEd.rightPen);
        } else {
            mCmd->Plonk(tp, mEd.leftPen);
        }
    }
    if (b & RIGHT) {
        if (mEd.useBrush) {
            mCmd->EraseBrush(tp, mEd.brush, mEd.rightPen);
        } else {
            mCmd->Plonk(tp, mEd.rightPen);
        }
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


static MapRect UpdateSelection(TilePoint const& anchor, TilePoint const& other)
{
    int xmin = std::min(anchor.x, other.x);
    int ymin = std::min(anchor.y, other.y);
    int xmax = std::max(anchor.x, other.x);
    int ymax = std::max(anchor.y, other.y);
    return MapRect(TilePoint(xmin,ymin), 1+xmax-xmin, 1+ymax-ymin);
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
    mSelection = UpdateSelection(mAnchor, mAnchor);
    view->SetCursor(mSelection);
}

void PickupTool::Move(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    if (!mLatch) {
        return;
    }
    TilePoint tp = mProj.ToTilePoint(pos);
    mSelection = UpdateSelection(mAnchor, tp);
    view->SetCursor(mSelection);
}

void PickupTool::Release(MapView* view, int mapNum, PixPoint const& pos, int b)
{
    if (mLatch) {
        TilePoint tp = mProj.ToTilePoint(pos);
        mSelection = UpdateSelection(mAnchor, tp);

        // Pick up brush
        Tilemap& map = mProj.maps[mapNum];
        mEd.brush = map.Copy(mSelection);
        mEd.useBrush = true;
        for (auto l : mEd.listeners) {
            l->EditorBrushChanged();
        }

        mLatch = 0;
        view->HideCursor();
        mEd.SetTool(TOOL_DRAW);
    }
}


void PickupTool::Reset()
{
    mLatch = 0;
}

