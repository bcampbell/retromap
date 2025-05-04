#pragma once

#include <set>
#include <vector>
#include <algorithm>

#include "model.h"

struct Proj;

// Base class for map editor widget in the GUI.
class MapEditor : public IModelListener {

public:
    MapEditor() = delete;
	MapEditor(Model& model);
    ~MapEditor();


    // set which map we're looking at
    void SetCurrentMap(int mapNum);
    int CurrentMap() const {return mCurMap;}

    void MapNavLinear(int delta);
    void MapNav2D(int dx, int dy);

    std::vector<int> const& SelectedEnts() const {return mSelectedEnts;}
    bool IsEntSelected(int endIdx) const;

    // IModelListener
    virtual void ProjCharsetModified();
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjNuke();
    virtual void ProjMapsInserted(int first, int count);
    virtual void ProjMapsRemoved(int first, int count);
    virtual void ProjEntsInserted(int mapNum, int entNum, int count);
    virtual void ProjEntsRemoved(int mapNum, int entNum, int count);
    virtual void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData);

    // To be called by GUI.
public:
    void SetSelectedEnts(std::vector<int> const& newSelection);
protected:
    // GUI calls these and the MapEditor uses them to drive the current Tool.
    void Press(PixPoint const& pt, int button);
    void Move(PixPoint const& pt, int button);
    void Release(PixPoint const& pt, int button);

public:
    // To be implemented by GUI.
    virtual void CurMapChanged() = 0 ;
    virtual void MapModified(MapRect const& dirty) = 0;
    virtual void EntsModified() = 0;
    virtual void SetCursor(MapRect const& area) = 0;
    virtual void HideCursor() = 0;
    virtual void EntSelectionChanged() = 0;
protected:
    Model& mModel;
    Proj& mProj;
    int mCurMap{0};
    std::vector<int> mSelectedEnts;
};

