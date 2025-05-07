#pragma once

#include <cstdio>
#include <cassert>

#include <QtWidgets/QWidget>

#include "model.h"


// Should be IView, not IModelListener, but need to update view interface.
class WorldWidget : public QWidget, public IModelListener {
    Q_OBJECT

public:
    WorldWidget() = delete;
	WorldWidget(QWidget* parent, Model& model);
    virtual ~WorldWidget();

    // IModelListener methods
    virtual void EditorPenChanged() {};
    virtual void EditorToolChanged() {};
    virtual void EditorBrushChanged() {};
    virtual void ProjCharsetModified() {};
    virtual void ProjMapModified(int mapNum, MapRect const& dirty) {};
    // Assume everything changed.
    virtual void ProjNuke() {};
    // Moves any following maps upward (assume all maps moved in memory!).
    virtual void ProjMapsInserted(int mapNum, int count) {};
    // Moves any following maps back (assume all maps have shifted in memory!).
    virtual void ProjMapsRemoved(int mapNum, int count) {};


    virtual void ProjEntsInserted(int mapNum, int entNum, int count) {};
    virtual void ProjEntsRemoved(int mapNum, int entNum, int count) {};
    virtual void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData) {};


    void setCurMap(int mapNum); // -1=none
    int curMap() const { return mCurMap; }
signals:
    void curMapChanged();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);

    void resizeEvent(QResizeEvent *event);
//    QSize sizeHint() const override;

private:
    void CalcLayout(int mapsacross);
    int PickMap(TilePoint const& p) const;

    Model& mModel;
    int mCurMap;
    // Maps laid out in the world (in tile coords)
    std::vector<MapRect> mLayout;
    MapRect mExtent;
    // Maps laid out in the widget (scaled to widget bounds)
    std::vector<QRectF> mOutlines;
};

