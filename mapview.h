#pragma once

class MapEditor;

#include "proj.h"

// Base interface for View part of MVP.
class MapView {
public:
    virtual void SetPresenter(MapEditor* presenter) = 0;
    virtual void SetMap(Tilemap *tilemap, Charset *charset, Palette *palette) = 0;
    virtual void MapModified(MapRect const& dirty) = 0;
    virtual void SetCursor(MapRect const& area) = 0;
    virtual void HideCursor() = 0;
    virtual ~MapView() = default;
};




