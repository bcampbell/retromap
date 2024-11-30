#pragma once

#include "proj.h"


// Self-contained functions which just draw stuff on a Tilemap.

MapRect Plonk(Tilemap &map, TilePoint const& pos, Cell const& pen, int drawFlags);
MapRect DrawRect(Tilemap& map, MapRect const& area, Cell const& pen, int drawFlags);
MapRect FloodFill(Tilemap& map, TilePoint const& start, Cell pen, int drawFlags);
MapRect DrawBrush(Tilemap& map, TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags);
MapRect EraseBrush(Tilemap& map, TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags);

