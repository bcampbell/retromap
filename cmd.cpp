#include "cmd.h"
#include "editor.h"
#include <cassert>
#include <algorithm>

void PlonkCmd::Do()
{
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];
    assert(map.IsValid(mPos));

    // swap em.
    Cell tmp = map.CellAt(mPos);
    map.CellAt(mPos) = mPen;
    mPen = tmp;

    mEd.modified = true;
    MapRect dirty(mPos, 1, 1);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, dirty);
    }
    mState = DONE;
}

void PlonkCmd::Undo()
{
    Do();
    mState = NOT_DONE;
}


void InsertMapsCmd::Do()
{
    auto dest = mEd.proj.maps.begin() + mPos;
    mEd.proj.maps.insert(dest, mNewMaps.begin(), mNewMaps.end());
    for (auto l : mEd.listeners) {
        l->ProjMapsInserted(mPos, mNewMaps.size());
    }

    mEd.modified = true;
    mState = DONE;
}

void InsertMapsCmd::Undo()
{
    auto& maps = mEd.proj.maps;
    maps.erase(maps.begin() + mPos, maps.begin() + mPos + mNewMaps.size());
    for (auto l : mEd.listeners) {
        l->ProjMapsRemoved(mPos, mNewMaps.size());
    }
    mState = NOT_DONE;
}


void ReplaceCharsetCmd::Do()
{
    std::swap(mEd.proj.charset, mTiles);
    for (auto l : mEd.listeners) {
        l->ProjCharsetModified();
    }
    mEd.modified = true;
    mState = DONE;
}

void ReplaceCharsetCmd::Undo()
{
    Do();
}
