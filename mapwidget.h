#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>

#include "proj.h"
#include "mapview.h"

class Tool;
class MapEditor;

// Implements the View part of MVP.
// Provides the GUI part.
// Handles rendering of map and overlayed stuff like cursors.
// Also handles the initial layer of input, but delgates most input events
// down to the Presenter layer.
// The Presenter layer calls back to the view when the map changes, or to
// update cursors.
class MapWidget : public QWidget, public MapView {
    Q_OBJECT

public:
    MapWidget() = delete;
	MapWidget(QWidget* parent);
    virtual ~MapWidget();

    // MapView methods
    virtual void SetPresenter(MapEditor* presenter);
    virtual void SetMap(Tilemap *tilemap, Charset *charset, Palette *palette);
    virtual void MapModified(MapRect const& dirty);
    virtual void SetCursor(MapRect const& area);
    virtual void HideCursor();

    void ShowGrid(bool yesno);
    bool IsGridShown() const {return mShowGrid;}
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;

private:
    MapEditor* mPresenter{nullptr};
    Tilemap* mTilemap{nullptr};
    Charset* mCharset{nullptr};
    Palette* mPalette{nullptr};
    QImage mBacking;
    int mZoom{3};
    bool mShowGrid{false};
    bool mCursorOn{false};
    MapRect mCursor;

    QRect FromMap(MapRect const& r) const;
    MapRect ToMap(QRectF const& r) const;

    void UpdateBacking(MapRect const& dirty);
    bool IsValidMap() const {return mTilemap && mCharset && mPalette;}
};

