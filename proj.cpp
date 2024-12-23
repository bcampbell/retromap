#include "proj.h"

#include <format>
#include <algorithm>

void MapRect::Merge(MapRect const& other) {
    if (IsEmpty()) {
        *this = other;
        return;
    }
    int xmin = std::min(x, other.x);
    int xmax = std::max(x + w, other.x + other.w);
    int ymin = std::min(y, other.y);
    int ymax = std::max(y + h, other.y + other.h);

    x = xmin;
    y = ymin;
    w = xmax - xmin;
    h = ymax - ymin;
}

void MapRect::Merge(TilePoint const& point)
{
    Merge(MapRect(point,1,1));
}

MapRect MapRect::Clip(MapRect const& r) const
{
    int left = std::max(0, r.x);
    int right = std::min(w, r.x + r.w);
    int top = std::max(0, r.y);
    int bottom = std::min(h, r.y + r.h);

    return MapRect(TilePoint(left,top), right - left, bottom - top);
}


Tilemap Tilemap::Copy(MapRect const& r) const
{
    Tilemap out;
    out.w = r.w;
    out.h = r.h;
    out.cells.resize(r.w * r.h);
    for (int y = 0; y < r.h; ++y) {
        Cell* dest = out.CellPtr(TilePoint(0,y));
        for (int x = 0; x < r.w; ++x) {
            // Slow and naive per-tile clipping, but nice and simple :-)
            TilePoint srcPos(r.x + x, r.y + y);
            if (Bounds().Contains(srcPos)) {
                *dest = CellAt(srcPos);
            }
            ++dest;
        }
    }
    return out;
}


static void PushU16LE(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(v & 0xFF);
    out.push_back(v >> 8);
}


void WriteProj(Proj const& proj, std::vector<uint8_t>& out)
{
    // Magic cookie/version
    out.push_back('r');
    out.push_back('1');

    // Write out maps.
    PushU16LE(out, (uint16_t)proj.maps.size());
    for (auto& map : proj.maps) {
        PushU16LE(out, (uint16_t)map.w);
        PushU16LE(out, (uint16_t)map.h);
        for(auto& cell : map.cells) {
            PushU16LE(out, cell.tile);
            out.push_back(cell.ink);
            out.push_back(cell.paper);
        }
    }

    // Write charset
    Charset const& tiles = proj.charset;
    {
        out.push_back((uint8_t)tiles.tw);
        out.push_back((uint8_t)tiles.th);
        PushU16LE(out, (uint16_t)tiles.ntiles);
        out.insert(out.end(), tiles.images.begin(), tiles.images.end());
    }

    // Write palette
    Palette const& palette = proj.palette;
    {
        PushU16LE(out, (uint16_t)palette.ncolours);
        out.insert(out.end(), palette.colours.begin(), palette.colours.end());
    }
}

bool ReadProj(Proj& proj, uint8_t const* p, uint8_t const* end)
{
    // Check magic cookie.
    if((end - p) < 2) { return false; }
    if (p[0] != 'r' || p[1] != '1') { return false; }
    p += 2;

    // Read maps.
    if((end - p) < 2) { return false; }
    int nmaps = (p[1]<<8) + p[0];
    p += 2;
    proj.maps.clear();
    for (int i = 0; i < nmaps; ++i) {
        Tilemap map;
        if((end - p) < 4) { return false; }
        map.w = (p[1]<<8) + p[0];
        p += 2;
        map.h = (p[1]<<8) + p[0];
        p += 2;
        // enough data for cells?
        if ((end-p) < map.w * map.h * (2 + 1 + 1)) { return false; }
        map.cells.resize(map.w * map.h);
        for (Cell& cell : map.cells) {
            cell.tile = (p[1]<<8) + p[0];
            p += 2;
            cell.ink = *p++;
            cell.paper = *p++;
        }
        proj.maps.push_back(map);
    }
    // Read charset.
    Charset& charset = proj.charset;
    {
        if ((end - p) < (1+1+2)) {return false;}
        charset.tw = (int)*p++;
        charset.th = (int)*p++;
        charset.ntiles = (int)((p[1]<<8) + p[0]);
        p += 2;
        
        // enough tile image data?
        int n = charset.tw * charset.th * charset.ntiles;
        if ((end - p) < n) { return false; }
        charset.images = std::vector<uint8_t>(p, p + n);
        p += n;
    }

    // Read palette
    Palette& palette = proj.palette;
    {
        if ((end - p) < 2) {return false;}
        palette.ncolours = (int)((p[1]<<8) + p[0]);
        p += 2;

        // enough colour data?
        int n = 4 * palette.ncolours;
        if ((end - p) < n) { return false; }
        palette.colours = std::vector<uint8_t>(p, p + n);
        p += n;
    }

    if (p != end) {
        printf("warning: leftover data...\n");
    }
    return true;
}



// Ent implementation
std::string Ent::ToString() const
{
    std::string out;
    for (auto const& a : attrs) {
        // TODO: percent-encode things!
        out += std::format("{}={} ", a.name, a.value);
    }
    return out;
}

void Ent::FromString(std::string const& s)
{
    auto it = s.begin();
    auto end = s.end();

    auto is_space = [](char c) -> bool {return c == ' ';};
    while(it != end) {
        const auto name = std::find_if_not(it, end, is_space); // skip space
        if (name == end) {
            break;
        }

        const auto equals = std::find(name, end, '=');
        if (equals == end) {
            break;
        }
        it = equals + 1;    // skip '='
        const auto val = std::find_if_not(it, end, is_space);   // skip space
        it = std::find_if(val, end, is_space);
        // blank values are OK.

        EntAttr attr;
        attr.name = std::string(name, equals);
        attr.value = std::string(val, it);

        attrs.push_back(attr);
    }
}

