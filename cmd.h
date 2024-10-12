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


class PlonkCmd : public Cmd
{
public:
    PlonkCmd() = delete;
    PlonkCmd(Editor& ed, int mapNum, TilePoint const& tp, Cell const& pen) :
        Cmd(ed), mMapNum(mapNum), mPos(tp), mPen(pen) {}
    virtual void Do();
    virtual void Undo();
private:
    int mMapNum;
    TilePoint mPos;
    Cell mPen;
};

