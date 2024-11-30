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


static Cell combine(Cell const& dest, Cell const& pen, int drawFlags)
{
    return Cell{
        drawFlags & DRAWFLAG_TILE ? pen.tile : dest.tile,
        drawFlags & DRAWFLAG_INK ? pen.ink : dest.ink,
        drawFlags & DRAWFLAG_PAPER ? pen.paper : dest.paper
    };
}

void MapDrawCmd::Plonk(TilePoint const& pos, Cell const& cell, int drawFlags)
{
    assert(State() == DONE);
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    assert(map.Bounds().Contains(pos));
    map.CellAt(pos) = combine(map.CellAt(pos), cell, drawFlags);
    // Tell everyone.
    mEd.modified = true;
    MapRect dirty(pos, 1, 1);
    mDamageExtent.Merge(dirty);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, dirty);
    }
}

void MapDrawCmd::DrawRect(MapRect const& area, Cell const& pen, int drawFlags)
{
    assert(State() == DONE);
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    // clip to map
    MapRect destRect = map.Bounds().Clip(area);
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // do it
    for (int y=0; y<srcRect.h; ++y) {
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            *dest = combine(*dest, pen, drawFlags);
            ++dest;
        }
    }
    // Tell everyone.
    mEd.modified = true;
    mDamageExtent.Merge(destRect);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, destRect);
    }
}

void MapDrawCmd::DrawBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags)
{
    assert(State() == DONE);
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    // clip brush area on map
    MapRect destRect = map.Bounds().Clip(MapRect(pos, brush.w, brush.h));
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.pos.x, srcRect.pos.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            Cell c = *src++;
            if (c.tile != transparent.tile) {
                *dest = combine(*dest, c, drawFlags);
            }
            ++dest;
        }
    }
    // Tell everyone.
    mEd.modified = true;
    mDamageExtent.Merge(destRect);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, destRect);
    }
}

void MapDrawCmd::EraseBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags)
{
    assert(State() == DONE);
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    // clip brush area on map
    MapRect destRect = map.Bounds().Clip(MapRect(pos, brush.w, brush.h));
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.pos.x, srcRect.pos.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            Cell c = *src++;
            if (c.tile != transparent.tile) {
                *dest = combine(*dest, transparent, drawFlags);
            }
            ++dest;
        }
    }
    // Tell everyone.
    mEd.modified = true;
    mDamageExtent.Merge(destRect);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, destRect);
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

