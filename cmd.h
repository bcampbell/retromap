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

    void Plonk(TilePoint const& pos, Cell const& cell);
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

