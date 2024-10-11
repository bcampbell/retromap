#pragma once

#include "editor.h"

struct PixPoint;
struct TilePoint;
struct Cell;
struct Proj;

class Tool
{
public:
    Tool() = delete;
    Tool(Editor& ed) : mEd(ed), mProj(ed.proj) {}
    virtual ~Tool() {}

    enum Button {
        LEFT = 0x01,
        RIGHT = 0x02,
    };

    virtual void Press(int mapNum, PixPoint const& pos, int b) {}
    virtual void Move(int mapNum, PixPoint const& pos, int b) {}
    virtual void Release(int mapNum, PixPoint const& pos, int b) {}
protected:
    Editor& mEd;
    Proj& mProj;
};


class DrawTool : public Tool
{
public:
    DrawTool() = delete;
    DrawTool(Editor& ed) : Tool(ed) {}
    virtual ~DrawTool() {}

    virtual void Press(int mapNum, PixPoint const& pos, int b);
    virtual void Move(int mapNum, PixPoint const& pos, int b);
    virtual void Release(int mapNum, PixPoint const& pos, int b);
private:
    void Plonk(int mapNum, TilePoint const& tp, Cell const& pen);
};

