#include "cmd.h"
#include "editor.h"
#include <cassert>


void PlonkCmd::Do()
{
    Proj& proj = mEd.proj;
    Tilemap& map = proj.maps[mMapNum];
    assert(map.IsValid(mPos));

    // swap em.
    Cell tmp = map.CellAt(mPos);
    map.CellAt(mPos) = mPen;
    mPen = tmp;

    mEd.modified = true;
    MapRect dirty(mPos, 1, 1);
    for (auto l : mEd.listeners) {
        l->ProjMapModified(mMapNum, dirty);
    }
    mState = DONE;
}

void PlonkCmd::Undo()
{
    Do();
    mState = NOT_DONE;
}

