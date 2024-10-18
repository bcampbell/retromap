#include "editor.h"
#include <set>

// Presenter part of MVP

class MapWidget;
struct Proj;
class Tool;

class MapEditor : public EditListener {

public:
    MapEditor() = delete;
	MapEditor(Editor& ed);
    ~MapEditor();

    // set which map we're looking at
    void AddView(MapWidget* view);
    void RemoveView(MapWidget* view);
    void SetCurrentMap(int mapNum);
    int CurrentMap() {return mCurMap;}

    // Called by view
    void Press(PixPoint const& pt, int button);
    void Move(PixPoint const& pt, int button);
    void Release(PixPoint const& pt, int button);

    // EditListener
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjMapsInserted(int first, int count);
    virtual void ProjMapsRemoved(int first, int count);
    virtual void ProjCharsetModified();

private:
    Editor& mEd;
    std::set<MapWidget*> mViews;
    Proj& mProj;
    Tool* mTool{nullptr};
    int mCurMap{0};
};

