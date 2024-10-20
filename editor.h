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
    virtual void ProjCharsetModified() {};
    virtual void ProjMapModified(int mapNum, MapRect const& dirty) {};
    // Assume everything changed.
    virtual void ProjNuke() {};
    // Moves any following maps upward.
    virtual void ProjMapsInserted(int mapNum, int count) {};
    // Moves any following maps back.
    virtual void ProjMapsRemoved(int mapNum, int count) {};
};


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

    // TODO: move this out into editor window.
    Cell leftPen;
    Cell rightPen;

    Tool* tool{nullptr};

    void AddCmd(Cmd* cmd);
    void Undo();
    void Redo();

    void SetTool(int toolKind);
};




