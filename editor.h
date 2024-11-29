#pragma once

#include <cstdint>
#include <string>
#include <set>
#include "proj.h"
#include "tool.h"

class Cmd;

class EditListener
{
public:
    virtual ~EditListener() = default;
    virtual void EditorPenChanged() {};
    virtual void EditorToolChanged() {};
    virtual void EditorBrushChanged() {};
    virtual void ProjCharsetModified() {};
    virtual void ProjMapModified(int mapNum, MapRect const& dirty) {};
    // Assume everything changed.
    virtual void ProjNuke() {};
    // Moves any following maps upward.
    virtual void ProjMapsInserted(int mapNum, int count) {};
    // Moves any following maps back.
    virtual void ProjMapsRemoved(int mapNum, int count) {};
};


// Bitflags for what to draw into cells
#define DRAWFLAG_TILE 0x01
#define DRAWFLAG_INK 0x02
#define DRAWFLAG_PAPER 0x04
#define DRAWFLAG_ALL (DRAWFLAG_TILE|DRAWFLAG_INK|DRAWFLAG_PAPER)

// Owns a Proj and holds all the editing state.
class Editor {
public:
    // start with a default proj
    Editor();
    ~Editor();

    Proj proj;
    bool modified{false};
    std::string mapFilename;
    std::string tilesetFilename;
    std::set<EditListener*> listeners;

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
};




