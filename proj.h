#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Cell {
    uint16_t tile{0};
    uint8_t ink{0};
    uint8_t paper{0};
};

struct Point
{
    int x{0};
    int y{0};
    Point(int xpos, int ypos) : x(xpos), y(ypos) {}
};

// A position on a map, in tile coords.
struct TilePoint : public Point
{
    TilePoint(int xpos, int ypos) : Point(xpos,ypos) {}
};

// A position on a map, in map pixels.
struct PixPoint : public Point
{
    PixPoint(int xpos, int ypos) : Point(xpos,ypos) {}
};


struct MapRect
{
    TilePoint pos;
    int w;
    int h;
};

struct Tilemap
{
    int w;
    int h;
    std::vector<Cell> cells;

    bool IsValid(TilePoint const& tp) const {
        return (tp.x >= 0 && tp.x < w && tp.y >= 0 && tp.y < h);
    }
    Cell& CellAt(TilePoint const& tp) {
        return cells[(tp.y * w) + tp.x];
    };
    const Cell& CellAt(TilePoint const& tp) const {
        return cells[(tp.y * w) + tp.x];
    };

    // Return a bounding rect for the map.
    const MapRect Bounds() const {
        return MapRect(TilePoint(0, 0), w, h);
    }
};

struct Tileset
{
    int tw;
    int th;
    int ntiles;
    std::vector<uint8_t> images; // 1byte/pixel

    uint8_t const* RawConst(int tile) const {
        return images.data() + (tw*th*tile);
    };
    uint8_t* Raw(int tile) {
        return images.data() + (tw*th*tile);
    };
};

struct Palette
{
    int ncolours;
    std::vector<uint8_t> colours;   // R,G,B,x
};


struct Proj
{
    std::vector<Tilemap> maps;
    Tileset tileset;
    Palette palette;

    TilePoint ToTilePoint(PixPoint const& pp) const {
        return TilePoint(pp.x / tileset.tw, pp.y / tileset.th);
    }

    PixPoint ToPixPoint(TilePoint const& mp) const {
        return PixPoint(mp.x * tileset.tw, mp.y * tileset.th);
    }
};



void WriteProj(Proj const& proj, std::vector<uint8_t>& out);
bool ReadProj(Proj& proj, uint8_t const* p, uint8_t const* end);

