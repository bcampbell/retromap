#include <set>
#include <vector>
#include <algorithm>

#include "model.h"

class IView;
struct Proj;

// MapPresenter is Presenter part of MVP
// Core class - no GUI code here, that's all in the view.
class MapPresenter : public IModelListener {

public:
    MapPresenter() = delete;
	MapPresenter(Model& ed);
    ~MapPresenter();


    // set which map we're looking at
    void AddView(IView* view);
    void RemoveView(IView* view);
    void SetCurrentMap(int mapNum);
    int CurrentMap() {return mCurMap;}

    void MapNavLinear(int delta);
    void MapNav2D(int dx, int dy);

    void SetSelectedEnts(std::vector<int> const& sel);
    std::vector<int> const& SelectedEnts() const {return mSelectedEnts;}
    bool IsEntSelected(int endIdx) const;

    // Called by view
    void Press(IView* view, PixPoint const& pt, int button);
    void Move(IView* view, PixPoint const& pt, int button);
    void Release(IView* view, PixPoint const& pt, int button);

    // IModelListener
    virtual void ProjCharsetModified();
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjNuke();
    virtual void ProjMapsInserted(int first, int count);
    virtual void ProjMapsRemoved(int first, int count);
    virtual void ProjEntsInserted(int mapNum, int entNum, int count);
    virtual void ProjEntsRemoved(int mapNum, int entNum, int count);
    virtual void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData);
private:
    Model& mEd;
    // Hmm. Should presenter and view be 1:1?
    std::set<IView*> mViews;
    Proj& mProj;
    int mCurMap{0};
    std::vector<int> mSelectedEnts;
};

