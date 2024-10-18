#include "mapeditor.h"
#include "mapwidget.h"
#include "tool.h"

MapEditor::MapEditor(Editor& ed) : mEd(ed), mProj(ed.proj)
{
    SetCurrentMap(0);
    mTool = new DrawTool(ed);
    ed.listeners.insert(this);
}

MapEditor::~MapEditor()
{
    mEd.listeners.erase(this);
    while(!mViews.empty()) {
        RemoveView(*mViews.begin());
    }
}

void MapEditor::AddView(MapWidget* view)
{
    mViews.insert(view);
    view->SetPresenter(this);
    view->SetMap(&mProj.maps[mCurMap], &mProj.charset, &mProj.palette);
}

void MapEditor::RemoveView(MapWidget* view)
{
    view->SetPresenter(nullptr);
    view->SetMap(nullptr, nullptr, nullptr);
    mViews.erase(view);
}

void MapEditor::SetCurrentMap(int mapNum)
{
    assert(mapNum >= 0 && mapNum < (int)mProj.maps.size());
    mCurMap = mapNum;
    for (auto view : mViews) {
        view->SetMap(&mProj.maps[mCurMap], &mProj.charset, &mProj.palette);
    }
}

// EditListener
void MapEditor::ProjMapModified(int mapNum, MapRect const& dirty)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->MapModified(dirty);
        }
    }
}


void MapEditor::ProjMapsInserted(int first, int count)
{
    if (mCurMap >= first) {
        SetCurrentMap(mCurMap + count);
    }
}

void MapEditor::ProjMapsRemoved(int first, int count)
{
    if (mCurMap >= first+count) {
        SetCurrentMap(mCurMap - count);
        return;
    }
    if (mCurMap >= first) {
        SetCurrentMap(std::min(int(mProj.maps.size()) - 1, first));
    }
}

void MapEditor::ProjCharsetModified()
{
    Tilemap& map = mProj.maps[mCurMap];
    ProjMapModified(mCurMap, map.Bounds());
}


void MapEditor::Press(PixPoint const& pt, int button)
{
    mTool->Press(mCurMap, pt, button);
}

void MapEditor::Move(PixPoint const& pt, int button)
{
    mTool->Move(mCurMap, pt, button);
}

void MapEditor::Release(PixPoint const& pt, int button)
{
    mTool->Release(mCurMap, pt, button);
}

