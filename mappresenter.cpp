#include "mappresenter.h"
#include "view.h"
#include "tool.h"
#include "model.h"

MapPresenter::MapPresenter(Model& ed) : mEd(ed), mProj(ed.proj)
{
    SetCurrentMap(0);
    ed.listeners.insert(this);
}

MapPresenter::~MapPresenter()
{
    mEd.listeners.erase(this);
    while(!mViews.empty()) {
        RemoveView(*mViews.begin());
    }
}

void MapPresenter::AddView(IView* view)
{
    mViews.insert(view);
    view->SetPresenter(this);
    view->SetMap(&mProj.maps[mCurMap], &mProj.charset, &mProj.palette);
}

void MapPresenter::RemoveView(IView* view)
{
    view->SetPresenter(nullptr);
    view->SetMap(nullptr, nullptr, nullptr);
    mViews.erase(view);
}

void MapPresenter::SetCurrentMap(int mapNum)
{
    assert(mapNum >= 0 && mapNum < (int)mProj.maps.size());
    mCurMap = mapNum;
    for (auto view : mViews) {
        view->SetMap(&mProj.maps[mCurMap], &mProj.charset, &mProj.palette);
    }
}

void MapPresenter::MapNavLinear(int delta)
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

void MapPresenter::MapNav2D(int dx, int dy)
{
    int w = 7;  // TODO: some setting, somewhere...
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

// ModelListenerListener
void MapPresenter::ProjMapModified(int mapNum, MapRect const& dirty)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->MapModified(dirty);
        }
    }
}

void MapPresenter::ProjNuke()
{
    int mapNum = mCurMap;
    if (mapNum >= (int)mProj.maps.size()) {
        assert(!mProj.maps.empty());
        mapNum = mProj.maps.size() - 1;
    }
    SetCurrentMap(mapNum);
}


void MapPresenter::ProjMapsInserted(int first, int count)
{
    int n = mCurMap >= first ? mCurMap : mCurMap + count;
    SetCurrentMap(n);
}

void MapPresenter::ProjMapsRemoved(int first, int count)
{
    int n = mCurMap;
   
    if (n >= first + count) {
        n -= count;
    }
    if (n >= first) {
        n = std::min(int(mProj.maps.size()) - 1, first);
    }

    SetCurrentMap(n);
}

void MapPresenter::ProjCharsetModified()
{
    Tilemap& map = mProj.maps[mCurMap];
    ProjMapModified(mCurMap, map.Bounds());
}

void MapPresenter::ProjEntsInserted(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}

void MapPresenter::ProjEntsRemoved(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}

void MapPresenter::ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData)
{
    if (mapNum == mCurMap) {
        for (auto view : mViews) {
            view->EntsModified();
        }
    }
}


void MapPresenter::Press(IView* view, PixPoint const& pt, int button)
{
    mEd.tool->Press(view, mCurMap, pt, button);
}

void MapPresenter::Move(IView* view, PixPoint const& pt, int button)
{
    mEd.tool->Move(view, mCurMap, pt, button);
}

void MapPresenter::Release(IView* view, PixPoint const& pt, int button)
{
    mEd.tool->Release(view, mCurMap, pt, button);
}

void MapPresenter::SetSelectedEnts(std::vector<int> const& sel) {
    mSelectedEnts = sel;
    for (auto view : mViews) {
        view->EntSelectionChanged();
    }
}

bool MapPresenter::IsEntSelected(int endIdx) const {
    return (std::find(mSelectedEnts.begin(), mSelectedEnts.end(), endIdx) != mSelectedEnts.end());
}
