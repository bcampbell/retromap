#pragma once

#include "proj.h"
#include <QPoint>

class QImage;
class QString;

void RenderCell(QImage& targ, QPoint pos, Tileset const& tileset, Palette const& palette, Cell const& pen);
bool ImportTileset(QString const& filename, Tileset& tileset, int tilew, int tileh);
void InitProj(Proj* proj);

bool WriteMap(Tilemap const& map, QString const& filename);
bool ReadMap(Tilemap& map, QString const& filename, int w, int h);

