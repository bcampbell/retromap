#include "proj.h"


void MapRect::Merge(MapRect const& other) {
    if (IsEmpty()) {
        *this = other;
        return;
    }
    int xmin = std::min(pos.x, other.pos.x);
    int xmax = std::max(pos.x + w, other.pos.x + other.w);
    int ymin = std::min(pos.y, other.pos.y);
    int ymax = std::max(pos.y + h, other.pos.y + other.h);

    pos.x = xmin;
    pos.y = ymin;
    w = xmax-xmin;
    h = ymax-ymin;
}

void MapRect::Merge(TilePoint const& point)
{
    Merge(MapRect(point,1,1));
}

MapRect MapRect::Clip(MapRect const& r) const
{
    int left = std::max(0, r.pos.x);
    int right = std::min(w, r.pos.x + r.w);
    int top = std::max(0, r.pos.y);
    int bottom = std::min(h, r.pos.y + r.h);

    return MapRect(TilePoint(left,top), right - left, bottom - top);
}


Tilemap Tilemap::Copy(MapRect const& r) const
{
    Tilemap out;
    out.w = r.w;
    out.h = r.h;
    out.cells.resize(r.w * r.h);
    TilePoint srcRowStart(r.pos);
    TilePoint destRowStart(0, 0);
    for (int y = 0; y < r.h; ++y) {
        Cell const* src = CellPtrConst(srcRowStart);
        Cell* dest = out.CellPtr(destRowStart);
        //Cell* backup = mBackup.CellPtr(TilePoint(0,y));
        for (int x = 0; x < r.w; ++x) {
            *dest++ = *src++;
        }
        ++srcRowStart.y;
        ++destRowStart.y;
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



