#pragma once

struct PixPoint;
struct TilePoint;
struct Cell;
struct Proj;
class MapEditor;
class MapDrawCmd;
class Model;

#define TOOL_DRAW 0
#define TOOL_PICKUP 1
//#define TOOL_BRUSH ???
#define TOOL_RECT 2
#define TOOL_FLOODFILL 3
#define TOOL_ENT 4
// Yes. Should be an enum. Patches welcome.

class Tool
{
public:

    Tool() = delete;
    Tool(Model& ed);
    virtual ~Tool();

    enum Button {
        LEFT = 0x01,
        RIGHT = 0x02,
    };

    virtual int Kind() const = 0;
    // TODO: could get mapNum from view?
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b) {}
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b) {}
    virtual void Release(MapEditor* view,int mapNum, PixPoint const& pos, int b) {}

    // Stop any in-progress operation and go back to initial state.
    virtual void Reset() {}
protected:
    Model& mEd;
    Proj& mProj;
};


// TODO: deal with changing mapNum
class DrawTool : public Tool
{
public:
    DrawTool() = delete;
    DrawTool(Model& ed) : Tool(ed) {}
    virtual ~DrawTool() {}

    virtual int Kind() const {return TOOL_DRAW;}
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Reset() {} //TODO!
private:
    TilePoint mPrevPos;
    MapDrawCmd* mCmd{nullptr};
    //void Plonk(int mapNum, TilePoint const& tp, Cell const& pen);
};

// Pick up a section of map to use as brush
class PickupTool : public Tool
{
public:
    PickupTool() = delete;
    PickupTool(Model& ed) : Tool(ed) {}
    virtual ~PickupTool() {}

    virtual int Kind() const {return TOOL_PICKUP;}
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Reset();
private:
    int mLatch{0};
    TilePoint mAnchor;
    MapRect mSelection;
};


class RectTool : public Tool
{
public:
    RectTool() = delete;
    RectTool(Model& ed) : Tool(ed) {}
    virtual ~RectTool() {}

    virtual int Kind() const {return TOOL_RECT;}
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Reset();
private:
    int mLatch{0};
    TilePoint mAnchor;
    MapRect mSelection;
};

class FloodFillTool : public Tool
{
public:
    FloodFillTool() = delete;
    FloodFillTool(Model& ed) : Tool(ed) {}
    virtual ~FloodFillTool() {}

    virtual int Kind() const {return TOOL_FLOODFILL;}
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Reset();
};


// Tool for moving/sizing ents
class EntTool : public Tool
{
public:
    EntTool() = delete;
    EntTool(Model& ed) : Tool(ed) {}
    virtual ~EntTool() {}

    virtual int Kind() const {return TOOL_ENT;}
    virtual void Press(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Move(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Release(MapEditor* view, int mapNum, PixPoint const& pos, int b);
    virtual void Reset();
private:
    enum Handle {NONE, MOVE, TOPLEFT, TOP, TOPRIGHT, RIGHT, BOTTOMRIGHT, BOTTOM, BOTTOMLEFT, LEFT};

    int mEnt{-1};
    Handle mHandle{NONE};
    PixPoint mAnchor;
};

