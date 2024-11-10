#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cassert>

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
    TilePoint() : Point(0, 0) {}
    TilePoint(int xpos, int ypos) : Point(xpos,ypos) {}
};

inline bool operator==(TilePoint const& a, TilePoint const& b)
    {return a.x == b.x && a.y == b.y;}


// A position on a map, in map pixels.
struct PixPoint : public Point
{
    PixPoint(int xpos, int ypos) : Point(xpos,ypos) {}
};


struct MapRect
{
    TilePoint pos;
    int w{0};
    int h{0};
    bool IsEmpty() const {return w == 0 && h == 0;}
    void Translate(TilePoint const& delta)
        {pos.x += delta.x; pos.y += delta.y;}
    void Merge(MapRect const& other);
    void Merge(TilePoint const& point);
    bool Contains(TilePoint const& point) const
    {
        return point.x >= pos.x && point.x < pos.x + w &&
            point.y >= pos.y && point.y < pos.y + h;
    }
    MapRect Clip(MapRect const& r) const;
};

struct Tilemap
{
    int w;
    int h;
    std::vector<Cell> cells;

    bool IsValid(TilePoint const& tp) const {
        return Bounds().Contains(tp);
    }
    Cell& CellAt(TilePoint const& tp) {
        assert(Bounds().Contains(tp));
        return cells[(tp.y * w) + tp.x];
    };
    const Cell& CellAt(TilePoint const& tp) const {
        assert(Bounds().Contains(tp));
        return cells[(tp.y * w) + tp.x];
    };

    Cell* CellPtr(TilePoint const& tp) {
        assert(Bounds().Contains(tp));
        return &cells[(tp.y * w) + tp.x];
    };
    Cell const* CellPtrConst(TilePoint const& tp) const {
        assert(Bounds().Contains(tp));
        return &cells[(tp.y * w) + tp.x];
    };

    // Return a bounding rect for the map.
    MapRect Bounds() const {
        return MapRect(TilePoint(0, 0), w, h);
    }

    // Return area r of as a new tilemap.
    // Any cells outside bounds of map will be zero.
    // (So r can go outside the map area).
    Tilemap Copy(MapRect const& r) const;
};

struct Charset
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
    Charset charset;
    Palette palette;

    TilePoint ToTilePoint(PixPoint const& pp) const {
        return TilePoint(pp.x / charset.tw, pp.y / charset.th);
    }

    PixPoint ToPixPoint(TilePoint const& mp) const {
        return PixPoint(mp.x * charset.tw, mp.y * charset.th);
    }
};



void WriteProj(Proj const& proj, std::vector<uint8_t>& out);
bool ReadProj(Proj& proj, uint8_t const* p, uint8_t const* end);

