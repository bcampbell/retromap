#pragma once

#include "proj.h"

// Base interface for View part of MVP.
class IView {
public:
    virtual void CurMapChanged() = 0;
    virtual void MapModified(MapRect const& dirty) = 0;
    virtual void EntsModified() = 0;
    virtual void SetCursor(MapRect const& area) = 0;
    virtual void HideCursor() = 0;
    virtual void EntSelectionChanged() =0;
    virtual void SetSelectedEnts(std::vector<int> newSelection) = 0;
    virtual ~IView() = default;
};





