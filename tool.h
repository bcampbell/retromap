#pragma once

#include "editor.h"

struct PixPoint;
struct TilePoint;
struct Cell;
struct Proj;
class MapView;
class MapDrawCmd;

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

    // TODO: could get mapNum from view?
    virtual void Press(MapView* view, int mapNum, PixPoint const& pos, int b) {}
    virtual void Move(MapView* view, int mapNum, PixPoint const& pos, int b) {}
    virtual void Release(MapView* view,int mapNum, PixPoint const& pos, int b) {}
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

    virtual void Press(MapView* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapView* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapView* view, int mapNum, PixPoint const& pos, int b);
private:
    TilePoint mPrevPos;
    MapDrawCmd* mCmd{nullptr};
    //void Plonk(int mapNum, TilePoint const& tp, Cell const& pen);
};

