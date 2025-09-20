#include "draw.h"
#include "model.h"

#include <algorithm>

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
    srcRect.x -= destRect.x;
    srcRect.y -= destRect.y;

    // do it
    for (int y=0; y<srcRect.h; ++y) {
        Cell *dest = map.CellPtr(TilePoint(destRect.x, destRect.y + y));
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

    auto match = [=](Cell const& a, Cell const& b) -> bool {
        if (drawFlags & DRAWFLAG_TILE) {
            if (a.tile != b.tile) {
                return false;
            }
        }
         if (drawFlags & DRAWFLAG_INK) {
            if (a.ink != b.ink) {
                return false;
            }
        }
         if (drawFlags & DRAWFLAG_PAPER) {
            if (a.paper != b.paper) {
                return false;
            }
        }
        // If we get this far, then it's a match.
        return true;
    };

    auto apply = [=](Cell& dest, Cell const& pen) {
        if (drawFlags & DRAWFLAG_TILE) {
            dest.tile = pen.tile;
        }
        if (drawFlags & DRAWFLAG_INK) {
            dest.ink= pen.ink;
        }
        if (drawFlags & DRAWFLAG_PAPER) {
            dest.paper= pen.paper;
        }
    };

    // Sample the tile at the start point for what we'll be replacing.
    Cell old = map.CellAt(start);
    if (match(old, pen)) {
        // already done...
        return damage;
    }

    std::vector<TilePoint> q;
    q.push_back(start);
    while(!q.empty())
    {
        TilePoint pt = q.back();
        q.pop_back();
        if (!match(map.CellAt(pt), old)) {
            continue;
        }

        // scan left and right to find span
        int y = pt.y;
        int l = pt.x;
        while (l > 0 && match(map.CellAt(TilePoint(l - 1, y)),old)) {
            --l;
        }
        int r = pt.x;
        while (r < map.w - 1 && match(map.CellAt(TilePoint(r + 1, y)), old)) {
            ++r;
        }

        // fill the span
        Cell* dest = map.CellPtr(TilePoint(l, y));
        int x;
        for (x = l; x <= r; ++x ) {
            apply(*dest++, pen);
        }

        // expand the damage box to include the affected span
        damage.Merge(MapRect(TilePoint(l, y), (r + 1 ) - l, 1));

        // add cells above the span to the queue
        y = pt.y - 1;
        if (y >= 0 )
        {
            for (x = l; x <= r; ++x)
            {
                if (match(map.CellAt(TilePoint(x, y)), old)) {
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
                if (match(map.CellAt(TilePoint(x, y)), old)) {
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
    srcRect.x -= destRect.x;
    srcRect.y -= destRect.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.x, srcRect.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.x, destRect.y + y));
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
    srcRect.x -= destRect.x;
    srcRect.y -= destRect.y;

    // copy
    for (int y=0; y<srcRect.h; ++y) {
        Cell const *src = brush.CellPtrConst(TilePoint(srcRect.x, srcRect.y + y));
        Cell *dest = map.CellPtr(TilePoint(destRect.x, destRect.y + y));
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


void HFlip(Tilemap& map)
{
    auto x0 = map.cells.begin();
    for (int y = 0; y < map.h; ++y) {
        auto x1 = x0 + map.w;
        std::reverse(x0, x1);
        x0 = x1;    // next line
    }
}

void VFlip(Tilemap& map)
{
    auto a = map.cells.begin();
    auto b = a + (map.w * (map.h - 1)); // start of last line
    for (int y = 0; y < map.h / 2; ++y) {
        std::swap_ranges(a, a + map.w, b);
        a += map.w;
        b -= map.w;
    }
}

