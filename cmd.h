#pragma once

#include "proj.h"

class Editor;

class Cmd
{
public:
    enum CmdState {NOT_DONE, DONE};
    Cmd(Editor& ed, CmdState initial = NOT_DONE) : mEd(ed), mState(initial) {}
    virtual ~Cmd() {}
    virtual void Do() = 0;
    virtual void Undo() = 0;
    CmdState State() const {return mState;}
protected:
    Editor& mEd;
    CmdState mState;
};

class MapDrawCmd : public Cmd
{
public:
    MapDrawCmd() = delete;
    MapDrawCmd(Editor& ed, int mapNum);

    // Add changes to cmd, applied immediately.
    void Plonk(TilePoint const& pos, Cell const& cell, int drawFlags);
    void DrawBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags);
    void EraseBrush(TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags);
    void DrawRect(MapRect const& area, Cell const& pen, int drawFlags);
    
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
    InsertMapsCmd(Editor& ed, std::vector<Tilemap> const& newMaps, int pos) :
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
    DeleteMapsCmd(Editor& ed, int beginMap, int endMap);
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
    ReplaceCharsetCmd(Editor& ed, Charset const& newTiles) :
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
    ResizeMapCmd(Editor& ed, int mapNum, MapRect newDimensions);
    virtual void Do();
    virtual void Undo();
private:
    void Swap();
    int mMapNum;
    Tilemap mOther;
};

class InsertEntsCmd : public Cmd
{
public:
    InsertEntsCmd() = delete;
    InsertEntsCmd(Editor& ed, int mapNum, std::vector<Ent> const& newEnts, int pos) :
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
    DeleteEntsCmd(Editor& ed, int mapNum, int pos, int count) :
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
    EditEntCmd(Editor& ed, int mapNum, Ent const& newData, int entNum) :
        Cmd(ed), mMapNum(mapNum), mEnt(newData), mEntNum(entNum) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    Ent mEnt;
    int mEntNum;
};

