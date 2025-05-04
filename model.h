#pragma once

#include <cstdint>
#include <string>
#include <set>
#include "proj.h"
#include "tool.h"

class Cmd;

// Callback interface for things that want to know about changes.
class IModelListener
{
public:
    virtual ~IModelListener() = default;
    virtual void EditorPenChanged() {};
    virtual void EditorToolChanged() {};
    virtual void EditorBrushChanged() {};
    virtual void ProjCharsetModified() {};
    virtual void ProjMapModified(int mapNum, MapRect const& dirty) {};
    // Assume everything changed.
    virtual void ProjNuke() {};
    // Moves any following maps upward (assume all maps moved in memory!).
    virtual void ProjMapsInserted(int mapNum, int count) {};
    // Moves any following maps back (assume all maps have shifted in memory!).
    virtual void ProjMapsRemoved(int mapNum, int count) {};


    virtual void ProjEntsInserted(int mapNum, int entNum, int count) {};
    virtual void ProjEntsRemoved(int mapNum, int entNum, int count) {};
    virtual void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData) {};
};


// Bitflags for what to draw into cells
#define DRAWFLAG_TILE 0x01
#define DRAWFLAG_INK 0x02
#define DRAWFLAG_PAPER 0x04
#define DRAWFLAG_ALL (DRAWFLAG_TILE|DRAWFLAG_INK|DRAWFLAG_PAPER)

// Owns a Proj and holds all the editing state.
class Model {
public:
    // start with a default proj
    Model ();
    ~Model();

    Proj proj;
    bool modified{false};
    std::string mapFilename;
    std::string tilesetFilename;
    std::set<IModelListener*> listeners;

	std::vector<Cmd*> undoStack;
	std::vector<Cmd*> redoStack;

    // Custom brush (0x0 = none)
    Tilemap brush;

    // TODO: move these out into editor window?
    Cell leftPen;
    Cell rightPen;
    bool useBrush{false};   // use brush for drawing?
    int drawFlags{DRAWFLAG_ALL}; // which parts of cell to draw to

    Tool* tool{nullptr};

    void AddCmd(Cmd* cmd);
    void Undo();
    void Redo();

    void SetTool(int toolKind);

    // Some accessors with asserts.
    Tilemap& GetMap(int mapNum) {
        assert(mapNum>=0 && mapNum < (int)proj.maps.size());
        return proj.maps[mapNum];
    }
    Ent& GetEnt(int mapNum, int entNum) {
        Tilemap& map = GetMap(mapNum);
        assert(entNum >=0 && entNum < (int)map.ents.size());
        return map.ents[entNum];
    }
};




