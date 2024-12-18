#include "cmd.h"
#include "editor.h"
#include <cassert>
#include <algorithm>

MapDrawCmd::MapDrawCmd(Editor& ed, int mapNum) :
    Cmd(ed, DONE),
    mMapNum(mapNum)
{
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];
    mDamageExtent = MapRect(TilePoint(0,0),0,0);
    mBackup = map;
}

void MapDrawCmd::AddDamage(MapRect const& damage)
{
    mEd.modified = true;
    mDamageExtent.Merge(damage);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, damage);
    }
}

void MapDrawCmd::Commit()
{
    // Trim down saved area to just that which was changed.
    // Just so we don't save a copy of the whole map for every edit!
    assert(State() == DONE);
    mBackupPos = mDamageExtent.pos;
    mBackup = mBackup.Copy(mDamageExtent);
}

void MapDrawCmd::Do()
{
    Swap();
    mState = DONE;
}

void MapDrawCmd::Undo()
{
    Swap();
    mState = NOT_DONE;
}

void MapDrawCmd::Swap()
{
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    MapRect r = mBackup.Bounds();
    TilePoint liveRowStart(mBackupPos);
    TilePoint backupRowStart(0, 0);
    for (int y = 0; y < r.h; ++y) {
        Cell* live = map.CellPtr(liveRowStart);
        Cell* backup = mBackup.CellPtr(backupRowStart);
        //Cell* backup = mBackup.CellPtr(TilePoint(0,y));
        for (int x = 0; x < r.w; ++x) {
            Cell tmp = *live;
            *live = *backup;
            *backup = tmp;
            //std::swap(*live, *backup);
            ++live;
            ++backup;
        }
        ++liveRowStart.y;
        ++backupRowStart.y;
    }
    MapRect affected(mBackup.Bounds());
    affected.Translate(mBackupPos);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, affected);
    }
}

//
// InsertMapsCmd
//
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

//
// DeleteMapsCmd
//
DeleteMapsCmd::DeleteMapsCmd(Editor& ed, int beginMap, int endMap) :
    Cmd(ed), mBeginMap(beginMap), mEndMap(endMap)
{
    int nMaps = (int)mEd.proj.maps.size();
    assert(beginMap >= 0 && beginMap < nMaps);
    assert(endMap >= 0 && endMap <= nMaps);
    assert(endMap >= beginMap);
}

void DeleteMapsCmd::Do()
{
    // Remove affected maps to mBackup.
    auto& maps = mEd.proj.maps;
    auto beginIt = maps.begin() + mBeginMap;
    auto endIt = maps.begin() + mEndMap;
    mBackup.reserve(endIt - beginIt);
    mBackup.clear();
    for (auto it = beginIt; it != endIt; ++it) {
        mBackup.push_back(std::move(*it));
    }
    maps.erase(beginIt, endIt);

    // Tell everyone.
    for (auto l : mEd.listeners) {
        l->ProjMapsRemoved(mBeginMap, mEndMap - mBeginMap);
    }

    mEd.modified = true;
    mState = DONE;
}

void DeleteMapsCmd::Undo()
{
    auto& maps = mEd.proj.maps;
    maps.insert(maps.begin() + mBeginMap, mBackup.begin(), mBackup.end());
    mBackup.clear();
    for (auto l : mEd.listeners) {
        l->ProjMapsInserted(mBeginMap, mBackup.size());
    }

    mEd.modified = true;
    mState = NOT_DONE;
}




//
// ReplaceCharsetCmd
//
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

//
// ResizeMapCmd
//
ResizeMapCmd::ResizeMapCmd(Editor& ed, int mapNum, MapRect r) :
    Cmd(ed), mMapNum(mapNum)
{
    assert(mapNum >=0 && mapNum < (int)ed.proj.maps.size());
    mOther = ed.proj.maps[mapNum].Copy(r);
}

void ResizeMapCmd::Swap()
{
    std::swap(mEd.proj.maps[mMapNum], mOther);
    for (auto l : mEd.listeners) {
        l->ProjNuke();
    }
    mEd.modified = true;
}

void ResizeMapCmd::Do()
{
    Swap();
    mState = DONE;
}

void ResizeMapCmd::Undo()
{
    Swap();
    mState = NOT_DONE;
}

