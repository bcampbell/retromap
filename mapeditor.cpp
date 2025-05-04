#include "mapeditor.h"
#include "tool.h"
#include "model.h"

MapEditor::MapEditor(Model& model) : mModel(model), mProj(model.proj), mCurMap(0)
{
    model.listeners.insert(this);
}

MapEditor::~MapEditor()
{
    mModel.listeners.erase(this);
}

void MapEditor::SetCurrentMap(int mapNum)
{
    assert(mapNum >= 0 && mapNum < (int)mProj.maps.size());
    mCurMap = mapNum;
    CurMapChanged();
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
void MapEditor::ProjMapModified(int mapNum, MapRect const& dirty)
{
    if (mapNum == mCurMap) {
        MapModified(dirty);
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
    int n = mCurMap >= first ? mCurMap : mCurMap + count;
    SetCurrentMap(n);
}

void MapEditor::ProjMapsRemoved(int first, int count)
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

void MapEditor::ProjCharsetModified()
{
    Tilemap& map = mProj.maps[mCurMap];
    ProjMapModified(mCurMap, map.Bounds());
}

void MapEditor::ProjEntsInserted(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        EntsModified();
    }
}

void MapEditor::ProjEntsRemoved(int mapNum, int entNum, int count)
{
    if (mapNum == mCurMap) {
        EntsModified();
    }
}

void MapEditor::ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData)
{
    if (mapNum == mCurMap) {
        EntsModified();
    }
}


void MapEditor::Press(PixPoint const& pt, int button)
{
    mModel.tool->Press(this, mCurMap, pt, button);
}

void MapEditor::Move(PixPoint const& pt, int button)
{
    mModel.tool->Move(this, mCurMap, pt, button);
}

void MapEditor::Release(PixPoint const& pt, int button)
{
    mModel.tool->Release(this, mCurMap, pt, button);
}

void MapEditor::SetSelectedEnts(std::vector<int> const& sel) {
    mSelectedEnts = sel;
    EntSelectionChanged();
}

bool MapEditor::IsEntSelected(int endIdx) const {
    return (std::find(mSelectedEnts.begin(), mSelectedEnts.end(), endIdx) != mSelectedEnts.end());
}
