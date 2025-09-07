#pragma once

#include "proj.h"

class Model;

class Cmd
{
public:
    enum CmdState {NOT_DONE, DONE};
    Cmd(Model& ed, CmdState initial = NOT_DONE) : mEd(ed), mState(initial) {}
    virtual ~Cmd() {}
    virtual void Do() = 0;
    virtual void Undo() = 0;
    CmdState State() const {return mState;}
protected:
    Model& mEd;
    CmdState mState;
};

class MapDrawCmd : public Cmd
{
public:
    MapDrawCmd() = delete;
    MapDrawCmd(Model& ed, int mapNum);

    void AddDamage(MapRect const& damage);
    void Commit();  // no more plonking!

    virtual void Do();
    virtual void Undo();
private:
    void Swap();
    int mMapNum;
    TilePoint mBackupPos;   // position of mBackup upon map
    Tilemap mBackup;
    MapRect mDamageExtent;
};


class InsertMapsCmd : public Cmd
{
public:
    InsertMapsCmd() = delete;
    InsertMapsCmd(Model& ed, std::vector<Tilemap> const& newMaps, int pos) :
        Cmd(ed), mNewMaps(newMaps), mPos(pos) {}
    virtual void Do();
    virtual void Undo();
private:
    std::vector<Tilemap> mNewMaps;
    int mPos;
};

class DeleteMapsCmd : public Cmd
{
public:
    DeleteMapsCmd() = delete;
    DeleteMapsCmd(Model& ed, int beginMap, int endMap);
    virtual void Do();
    virtual void Undo();
private:
    std::vector<Tilemap> mBackup;
    int mBeginMap;
    int mEndMap;
};

class ReplaceCharsetCmd : public Cmd
{
public:
    ReplaceCharsetCmd() = delete;
    ReplaceCharsetCmd(Model& ed, Charset const& newTiles) :
        Cmd(ed), mTiles(newTiles) {}
    virtual void Do();
    virtual void Undo();
private:
    Charset mTiles;
};

// Change the size of a given map.
class ResizeMapCmd : public Cmd
{
public:
    ResizeMapCmd() = delete;
    ResizeMapCmd(Model& ed, int mapNum, MapRect newDimensions);
    virtual void Do();
    virtual void Undo();
private:
    void Swap();
    int mMapNum;
    Tilemap mOther;
};

// Exchange two maps.
class ExchangeMapsCmd : public Cmd
{
public:
    ExchangeMapsCmd() = delete;
    ExchangeMapsCmd(Model& ed, int map1, int map2);
    virtual void Do();
    virtual void Undo();
private:
    void Swap();
    int mMap1;
    int mMap2;
};

class InsertEntsCmd : public Cmd
{
public:
    InsertEntsCmd() = delete;
    InsertEntsCmd(Model& ed, int mapNum, std::vector<Ent> const& newEnts, int pos) :
        Cmd(ed), mMapNum(mapNum), mPos(pos), mNewEnts(newEnts) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    int mPos;
    std::vector<Ent> mNewEnts;
};

class DeleteEntsCmd : public Cmd
{
public:
    DeleteEntsCmd() = delete;
    DeleteEntsCmd(Model& ed, int mapNum, int pos, int count) :
        Cmd(ed), mMapNum(mapNum), mPos(pos), mCount(count) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    int mPos;
    int mCount;
    std::vector<Ent> mBackup;   // Store removed Ents for undo.
};


class EditEntCmd : public Cmd
{
public:
    EditEntCmd() = delete;
    EditEntCmd(Model& ed, int mapNum, Ent const& newData, int entNum) :
        Cmd(ed), mMapNum(mapNum), mEnt(newData), mEntNum(entNum) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    Ent mEnt;
    int mEntNum;
};


// On the given map, swap all occurances of tile a with tile b.
class RemapTilesCmd : public Cmd
{
public:
    RemapTilesCmd() = delete;
    RemapTilesCmd(Model& ed, int mapNum, uint16_t tileA, uint16_t tileB) :
        Cmd(ed), mMapNum(mapNum), mTileA(tileA), mTileB(tileB) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    uint16_t mTileA;
    uint16_t mTileB;
};

// On the given map, swap all occurances of ink a with ink b.
class RemapInkCmd : public Cmd
{
public:
    RemapInkCmd() = delete;
    RemapInkCmd(Model& ed, int mapNum, uint8_t inkA, uint8_t inkB) :
        Cmd(ed), mMapNum(mapNum), mInkA(inkA), mInkB(inkB) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    uint8_t mInkA;
    uint8_t mInkB;
};

