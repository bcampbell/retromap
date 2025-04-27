#pragma once

class MapPresenter;

#include "proj.h"

// Base interface for View part of MVP.
class IView {
public:
    virtual void SetPresenter(MapPresenter* presenter) = 0;
    virtual MapPresenter& Presenter() = 0;
    virtual void SetMap(Tilemap *tilemap, Charset *charset, Palette *palette) = 0;
    virtual void MapModified(MapRect const& dirty) = 0;
    virtual void EntsModified() = 0;
    virtual void SetCursor(MapRect const& area) = 0;
    virtual void HideCursor() = 0;
    virtual void EntSelectionChanged() =0;
    virtual void SetSelectedEnts(std::vector<int> newSelection) = 0;
    virtual ~IView() = default;
};





