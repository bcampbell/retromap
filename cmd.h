#pragma once

class Cmd
{
public:
    enum State {NOT_APPLIED, APPLIED};
    Cmd(Editor& ed, State inital = NOT_DONE) : mEd(ed), mState(initial) {}
    virtual ~Cmd() {}
    virtual void Do() = 0;
    virtual void Undo() = 0;
protected:
    Editor& mEd;
    State mState;
};

