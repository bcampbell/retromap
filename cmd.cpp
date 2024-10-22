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

void MapDrawCmd::Plonk(TilePoint const& pos, Cell const& cell)
{
    assert(State() == DONE);
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];

    assert(map.Bounds().Contains(pos));
    map.CellAt(pos) = cell;
    // Tell everyone.
    mEd.modified = true;
    MapRect dirty(pos, 1, 1);
    mDamageExtent.Merge(dirty);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, dirty);
    }
}

void MapDrawCmd::DrawBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent)
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
                *dest = c;
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

void MapDrawCmd::EraseBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent)
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
                *dest = transparent;
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
