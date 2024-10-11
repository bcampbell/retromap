#pragma once

#include <cstdint>
#include <string>
#include <set>
#include "proj.h"


class EditListener
{
public:
    virtual ~EditListener() = default;
    virtual void ProjPenChanged() {};
    virtual void ProjTilesetModified() {};
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
    ~Editor() {}

    Proj proj;
    bool modified{false};
    std::string mapFilename;
    std::string tilesetFilename;
    std::set<EditListener*> listeners;

    // TODO: move this out into editor window.
    Cell leftPen;
    Cell rightPen;
};




