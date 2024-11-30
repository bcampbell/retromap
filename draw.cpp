#include "draw.h"
#include "editor.h"

static Cell combine(Cell const& dest, Cell const& pen, int drawFlags)
{
    return Cell{
        drawFlags & DRAWFLAG_TILE ? pen.tile : dest.tile,
        drawFlags & DRAWFLAG_INK ? pen.ink : dest.ink,
        drawFlags & DRAWFLAG_PAPER ? pen.paper : dest.paper
    };
}

MapRect Plonk(Tilemap &map, TilePoint const& pos, Cell const& pen, int drawFlags)
{
    assert(map.Bounds().Contains(pos));
    map.CellAt(pos) = combine(map.CellAt(pos), pen, drawFlags);
    return MapRect(pos, 1, 1);
}

MapRect DrawRect(Tilemap& map, MapRect const& area, Cell const& pen, int drawFlags)
{
    // clip to map
    MapRect destRect = map.Bounds().Clip(area);
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // do it
    for (int y=0; y<srcRect.h; ++y) {
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            *dest = combine(*dest, pen, drawFlags);
            ++dest;
        }
    }
    return destRect;
}





// Do floodfill, return damaged area.
// TODO: work out how drawFlags should work here.
MapRect FloodFill(Tilemap& map, TilePoint const& start, Cell pen, int drawFlags)
{
    MapRect damage;

    // Sample the tile at the start point for what we'll be replacing.
    Cell old = map.CellAt(start);
    if (old.tile == pen.tile) {
        // already done...
        return damage;
    }

    std::vector<TilePoint> q;
    q.push_back(start);
    while(!q.empty())
    {
        TilePoint pt = q.back();
        q.pop_back();
        if (map.CellAt(pt).tile != old.tile) {
            continue;
        }

        // scan left and right to find span
        int y = pt.y;
        int l = pt.x;
        while (l > 0 && map.CellAt(TilePoint(l - 1, y)).tile == old.tile) {
            --l;
        }
        int r = pt.x;
        while (r < map.w - 1 && map.CellAt(TilePoint(r + 1, y)).tile == old.tile) {
            ++r;
        }

        // fill the span
        Cell* dest = map.CellPtr(TilePoint(l, y));
        int x;
        for (x = l; x <= r; ++x ) {
            *dest++ = pen;
        }

        // expand the damage box to include the affected span
        damage.Merge(MapRect(TilePoint(l, y), (r + 1 ) - l, 1));

        // add cells above the span to the queue
        y = pt.y - 1;
        if (y >= 0 )
        {
            for (x = l; x <= r; ++x)
            {
                if (map.CellAt(TilePoint(x, y)).tile == old.tile) {
                    q.push_back(TilePoint(x, y));
                }
            }
        }

        // add pixels below the span to the queue
        y = pt.y + 1;
        if (y < map.h)
        {
            for (x = l; x <= r; ++x)
            {
                if (map.CellAt(TilePoint(x, y)).tile == old.tile) {
                    q.push_back(TilePoint(x, y));
                }
            }
        }
    }
    return damage;
}


MapRect DrawBrush(Tilemap& map, TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags)
{
    // clip brush area on map
    MapRect destRect = map.Bounds().Clip(MapRect(pos, brush.w, brush.h));
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.pos.x, srcRect.pos.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            Cell c = *src++;
            if (c.tile != transparent.tile) {
                *dest = combine(*dest, c, drawFlags);
            }
            ++dest;
        }
    }
    return destRect;
}

MapRect EraseBrush(Tilemap& map, TilePoint const& pos, Tilemap const& brush, Cell const& transparent, int drawFlags)
{
    // clip brush area on map
    MapRect destRect = map.Bounds().Clip(MapRect(pos, brush.w, brush.h));
    // transform clipped area into brush space
    MapRect srcRect = destRect;
    srcRect.pos.x -= destRect.pos.x;
    srcRect.pos.y -= destRect.pos.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.pos.x, srcRect.pos.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.pos.x, destRect.pos.y + y));
        for (int x=0; x<srcRect.w; ++x) {
            Cell c = *src++;
            if (c.tile != transparent.tile) {
                *dest = combine(*dest, transparent, drawFlags);
            }
            ++dest;
        }
    }
    return destRect;
}


