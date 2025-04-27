#include "cmd.h"
#include "model.h"
#include <cassert>
#include <algorithm>

MapDrawCmd::MapDrawCmd(Model& ed, int mapNum) :
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
    mBackupPos = mDamageExtent.Pos();
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
DeleteMapsCmd::DeleteMapsCmd(Model& ed, int beginMap, int endMap) :
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
    mState = NOT_DONE;
}

//
// ResizeMapCmd
//
ResizeMapCmd::ResizeMapCmd(Model& ed, int mapNum, MapRect r) :
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


//
// ExchangeMapsCmd
//
ExchangeMapsCmd::ExchangeMapsCmd(Model& ed, int map1, int map2) :
    Cmd(ed), mMap1(map1), mMap2(map2)
{
    assert(map1 >=0 && map1 < (int)ed.proj.maps.size());
    assert(map2 >=0 && map2 < (int)ed.proj.maps.size());
}

void ExchangeMapsCmd::Swap()
{
    std::swap(mEd.proj.maps[mMap1], mEd.proj.maps[mMap2]);
    for (auto l : mEd.listeners) {
        l->ProjNuke();
    }
    mEd.modified = true;
}

void ExchangeMapsCmd::Do()
{
    Swap();
    mState = DONE;
}

void ExchangeMapsCmd::Undo()
{
    Swap();
    mState = NOT_DONE;
}

//
// InsertEntsCmd
//
void InsertEntsCmd::Do()
{
    Tilemap& map = mEd.GetMap(mMapNum);

    assert(mPos >= 0 && mPos <= (int)map.ents.size());
    auto dest = map.ents.begin() + mPos;
    map.ents.insert(dest, mNewEnts.begin(), mNewEnts.end());
    for (auto l : mEd.listeners) {
        l->ProjEntsInserted(mMapNum, mPos, mNewEnts.size());
    }

    mEd.modified = true;
    mState = DONE;
}

void InsertEntsCmd::Undo()
{
    Tilemap& map = mEd.proj.maps[mMapNum];
    map.ents.erase(map.ents.begin() + mPos, map.ents.begin() + mPos + mNewEnts.size());
    for (auto l : mEd.listeners) {
        l->ProjEntsRemoved(mMapNum, mPos, mNewEnts.size());
    }
    mState = NOT_DONE;
}

//
// DeleteEntsCmd
//
void DeleteEntsCmd::Do()
{
    auto& ents = mEd.GetMap(mMapNum).ents;

    assert(mPos >= 0 && mPos <= (int)ents.size());
    assert((mPos + mCount) <= (int)ents.size());

    // Remove affected ents to mBackup.
    auto beginIt = ents.begin() + mPos;
    auto endIt = ents.begin() + mPos + mCount;
    mBackup.reserve(endIt - beginIt);
    mBackup.clear();
    std::move(beginIt, endIt, std::back_inserter(mBackup));
    ents.erase(beginIt, endIt);

    // Tell everyone.
    for (auto l : mEd.listeners) {
        l->ProjEntsRemoved(mMapNum, mPos, mCount);
    }

    mEd.modified = true;
    mState = DONE;
}

void DeleteEntsCmd::Undo()
{
    assert(mCount == (int)mBackup.size());
    auto& ents = mEd.GetMap(mMapNum).ents;
    ents.insert(ents.begin() + mPos, mBackup.begin(), mBackup.end());
    mBackup.clear();
    for (auto l : mEd.listeners) {
        l->ProjEntsInserted(mMapNum, mPos, mCount);
    }

    mEd.modified = true;
    mState = NOT_DONE;
}


//
// EditEntCmd
//
void EditEntCmd::Do()
{
    assert(mMapNum >=0 && mMapNum < (int)mEd.proj.maps.size());
    Tilemap& map = mEd.proj.maps[mMapNum];

    std::swap(map.ents[mEntNum], mEnt);
    for (auto l : mEd.listeners) {
        l->ProjEntChanged(mMapNum, mEntNum, mEnt, map.ents[mEntNum]);
    }

    mEd.modified = true;
    mState = DONE;
}

void EditEntCmd::Undo()
{
    Do();
    mState = NOT_DONE;
}

