#include "mapeditor.h"
#include "mapwidget.h"
#include "tool.h"

MapEditor::MapEditor(Editor& ed) : mEd(ed), mProj(ed.proj)
{
    SetCurrentMap(0);
    ed.listeners.insert(this);
}

MapEditor::~MapEditor()
{
    mEd.listeners.erase(this);
    while(!mViews.empty()) {
        RemoveView(*mViews.begin());
    }
}

void MapEditor::AddView(MapView* view)
{
    mViews.insert(view);
    view->SetPresenter(this);
    view->SetMap(&mProj.maps[mCurMap], &mProj.charset, &mProj.palette);
}

void MapEditor::RemoveView(MapView* view)
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

void MapEditor::MapNavLinear(int delta)
{
    int numMaps = (int)mProj.maps.size();
    int n = mCurMap + delta;
    while(n < 0) {
        n += numMaps;
    }
    while(n >= numMaps) {
        n -= numMaps;
    }
    SetCurrentMap(n);
}

void MapEditor::MapNav2D(int dx, int dy)
{
    int w = 5;  // TODO: some setting, somewhere...
    int h = (int)mProj.maps.size() / w;
    int x = (mCurMap % w) + dx;
    int y = (mCurMap / w) + dy;
    if(x < 0) {x += w;}
    if(x >= w) {x -= w;}
    if(y < 0) {y += h;}
    if(y >= h) {y -= h;}
    int n = y*w + x;
    if (n >= 0 && n < (int)mProj.maps.size()) {
        SetCurrentMap(n);
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

void MapEditor::ProjNuke()
{
    int mapNum = mCurMap;
    if (mapNum >= (int)mProj.maps.size()) {
        assert(!mProj.maps.empty());
        mapNum = mProj.maps.size() - 1;
    }
    SetCurrentMap(mapNum);
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

void MapEditor::ProjEntsInserted(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}

void MapEditor::ProjEntsRemoved(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}

void MapEditor::ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}


void MapEditor::Press(MapView* view, PixPoint const& pt, int button)
{
    mEd.tool->Press(view, mCurMap, pt, button);
}

void MapEditor::Move(MapView* view, PixPoint const& pt, int button)
{
    mEd.tool->Move(view, mCurMap, pt, button);
}

void MapEditor::Release(MapView* view, PixPoint const& pt, int button)
{
    mEd.tool->Release(view, mCurMap, pt, button);
}

