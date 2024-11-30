#include "editor.h"
#include <set>


class MapView;
struct Proj;

// MapEditor is Presenter part of MVP
// Core class - no GUI code here, that's all in the view.
class MapEditor : public EditListener {

public:
    MapEditor() = delete;
	MapEditor(Editor& ed);
    ~MapEditor();


    // set which map we're looking at
    void AddView(MapView* view);
    void RemoveView(MapView* view);
    void SetCurrentMap(int mapNum);
    int CurrentMap() {return mCurMap;}

    void MapNavLinear(int delta);
    void MapNav2D(int dx, int dy);

    // Called by view
    void Press(MapView* view, PixPoint const& pt, int button);
    void Move(MapView* view, PixPoint const& pt, int button);
    void Release(MapView* view, PixPoint const& pt, int button);

    // EditListener
    virtual void ProjCharsetModified();
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjNuke();
    virtual void ProjMapsInserted(int first, int count);
    virtual void ProjMapsRemoved(int first, int count);

private:
    Editor& mEd;
    std::set<MapView*> mViews;
    Proj& mProj;
    int mCurMap{0};
};

