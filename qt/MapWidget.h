#pragma once

#include <cstdio>
#include <cassert>
#include <vector>

#include <QtWidgets/QWidget>
#include <QImage>

#include "proj.h"
#include "view.h"
#include "mappresenter.h"

class Tool;

// Implements the View part of MVP.
// Provides the GUI part.
// Handles rendering of map and overlayed stuff like cursors.
// Also handles the initial layer of input, but delgates most input events
// down to the Presenter layer.
// The Presenter layer calls back to the view when the map changes, or to
// update cursors.
class MapWidget : public QWidget, public IView {
    Q_OBJECT

public:
    MapWidget() = delete;
	MapWidget(QWidget* parent, Model& model);
    virtual ~MapWidget();

    // IView methods
    virtual void CurMapChanged();
    virtual void MapModified(MapRect const& dirty);
    virtual void EntsModified();
    virtual void SetCursor(MapRect const& area);
    virtual void HideCursor();
    virtual void EntSelectionChanged();
    virtual void SetSelectedEnts(std::vector<int> newSelection);

    void ShowGrid(bool yesno);
    bool IsGridShown() const {return mShowGrid;}

    // called by mainwindow?
    int CurrentMap() const {
        return mPresenter.CurrentMap();
    }

    void MapNavLinear(int delta) {
        mPresenter.MapNavLinear(delta);
    }
    void MapNav2D(int dx, int dy) {
        mPresenter.MapNav2D(dx, dy);
    }

    std::vector<int> const& SelectedEnts() const {
        return mPresenter.SelectedEnts();
    }
signals:
    void cursorChanged(MapRect const& cursor);
    // Emitted when presenter ent selection is changed by mapwidget.
    void entSelectionChanged();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;

private:
    Model& mModel;
    MapPresenter mPresenter;
    QImage mBacking;
    int mZoom{3};
    bool mShowGrid{false};
    bool mCursorOn{false};
    MapRect mCursor;

    Tilemap& Map() const {return mModel.proj.maps[CurrentMap()];}
    QRect FromMap(MapRect const& r) const;
    MapRect ToMap(QRectF const& r) const;

    void UpdateBacking(MapRect const& dirty);
};

