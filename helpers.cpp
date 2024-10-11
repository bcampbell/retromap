#include "helpers.h"

#include <QFile>
#include <QImage>
#include <QIODevice>
#include <QString>
#include <QSaveFile>

void RenderCell(QImage& targ, QPoint pos, Tileset const& tileset, Palette const& palette, Cell const& pen)
{
    //printf("renderCell(%d %d tile:%d ink:%d paper:%d)\n", pos.x(), pos.y(), pen.tile, pen.ink, pen.paper);
    uint8_t const* ink = &(palette.colours[pen.ink*4]);
    uint8_t const* paper = &(palette.colours[pen.paper*4]);
    uint8_t const* src = tileset.RawConst(pen.tile);
    for (int cy = 0; cy < tileset.th; ++cy) {
        uchar* dest = targ.scanLine(pos.y() + cy) + (pos.x() * 4);
        for (int cx = 0; cx < tileset.th; ++cx) {
            uint8_t pix = *src++;
            if (pix == 0) {
                *dest++ = paper[0];
                *dest++ = paper[1];
                *dest++ = paper[2];
                *dest++ = 255;
            } else {
                *dest++ = ink[0];
                *dest++ = ink[1];
                *dest++ = ink[2];
                *dest++ = 255;
            }
        }
    }
}


bool ImportTileset(QString const& filename, Tileset& tileset, int tilew, int tileh)
{
    QImage im;
    if (!im.load(filename)) {
        return false;
    }

    if (im.format() != QImage::Format_Indexed8) {
        im = im.convertToFormat(QImage::Format_Indexed8);
    }

    // How many tiles can we squeeze out of this image?
    int gridw = im.width()/tilew;
    int gridh = im.height()/tileh;
    if (gridw < 1 || gridh < 1 ) {
        return false;   // No tiles!
    }

    tileset.ntiles = gridw * gridh;
    tileset.tw = tilew;
    tileset.th = tileh;
    tileset.images.resize(tileset.tw * tileset.th * tileset.ntiles); // 1byte/pixel

    int tile = 0;
    for (int ty = 0; ty < gridh; ++ty) {
        for (int tx = 0; tx < gridw; ++tx) {
            uint8_t* dest = tileset.Raw(tile);
            ++tile;
            // copy a tile
            for (int y = 0; y < tileset.th; ++y) {
                uint8_t const* src = im.scanLine(ty * tileset.th + y) + (1 * tx * tileset.tw);
                for (int x = 0; x < tileset.tw; ++x) {
                    *dest++ = *src++;
                }
            }
        }
    }
    return true;
}



void InitProj(Proj* proj)
{
    // https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_graphics#C-64
    static const uint8_t c64palette[3*16] = {
        0,    0,    0,   // Black
        255,  255,  255, // White
        136,   57,   50, // Red
        103,  182,  189, // Cyan
        139,   63,  150, // Purple
         85,  160,   73, // Green
         64,   49,  141, // Blue
        191,  206,  114, // Yellow
        139,   84,   41, // Orange
         87,   66,    0, // Brown
        184,  105,   98, // Light Red
         80,   80,   80, // Dark Grey
        120,  120,  120, // Grey
        148,  224,  137, // Light Green
        120,  105,  196, // Light Blue
        159,  159,  159, // Light Grey
    };

    Tilemap map;
    map.w = 40;
    map.h = 25;
    map.cells.resize(map.w * map.h);
    proj->maps.push_back(map);

    if (!ImportTileset("c64charset.png", proj->tileset, 8, 8)) {
        printf("Poop. load fail.\n");
        Tileset& t = proj->tileset;
        t.tw = 8;
        t.th = 8;
        t.ntiles = 2;
        t.images.resize(t.tw * t.th * t.ntiles);
        uint8_t* dest = t.Raw(0);
        for (int i = 0; i < 8 * 8; ++i) {
            *dest++ = 0;
        }
        for (int i = 0; i < 8 * 8; ++i) {
            *dest++ = 1;
        }
    }

    Palette& pal = proj->palette;
    pal.ncolours = 16;
    pal.colours.resize(pal.ncolours * 4);
    for (int i = 0; i < pal.ncolours; ++i) {
        pal.colours[i * 4 + 0] = c64palette[i * 3 + 0];
        pal.colours[i * 4 + 1] = c64palette[i * 3 + 1];
        pal.colours[i * 4 + 2] = c64palette[i * 3 + 2];
        pal.colours[i * 4 + 3] = 255;
    }
}


// Safely write map out to a file.
// Tiles first, then colours. No metadata.
bool WriteMap(Tilemap const& map, QString const& filename)
{
    QSaveFile out(filename);
    if (!out.open(QIODevice::WriteOnly)) {
        return false;
    }
    for (int y = 0; y < map.h; ++y) {
        std::vector<uint8_t> buf;
        for (int x = 0; x < map.w; ++x) {
            Cell const& c = map.CellAt(TilePoint{x, y});
            // clip tile to 8 bit
            buf.push_back((uint8_t)c.tile);
        }
        out.write((const char*)buf.data(), (qint64)buf.size());
    }
    for (int y = 0; y < map.h; ++y) {
        std::vector<uint8_t> buf;
        for (int x = 0; x < map.w; ++x) {
            Cell const& c = map.CellAt(TilePoint{x, y});
            // clip colours to 4 bit, paper in upper nibble, ink in lower.
            // (c64 colour ram is only 4bit, so paper will be ignored).
            uint8_t colour = ((c.paper & 0x0f)<<4) | (c.ink & 0x0f);
            buf.push_back(colour);
        }
        out.write((const char*)buf.data(), (qint64)buf.size());
    }

    return out.commit();
}

bool ReadMap(Tilemap& map, QString const& filename, int w, int h)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    map.w = w;
    map.h = h;
    map.cells.resize(map.w * map.h);

    // tiles
    for (int y = 0; y < h; ++y) {
        std::vector<uint8_t> buf(w);
        qint64 n = file.read((char*)buf.data(), (qint64)buf.size());
        if (n != (qint64)buf.size()) {
            return false;
        }
        for (int x = 0; x < w; ++x) {
            map.CellAt(TilePoint{x, y}).tile = buf[x];
        }
    }
    // colours
    for (int y = 0; y < h; ++y) {
        std::vector<uint8_t> buf(w);
        qint64 n = file.read((char*)buf.data(), (qint64)buf.size());
        if (n != (qint64)buf.size()) {
            return false;
        }
        for (int x = 0; x < w; ++x) {
            map.CellAt(TilePoint{x, y}).ink = buf[x] & 0x0f;
            map.CellAt(TilePoint{x, y}).paper = (buf[x]>>4) & 0x0f;
        }
    }
    return true;
}

