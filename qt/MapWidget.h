#pragma once

#include <cstdio>
#include <cassert>
#include <vector>

#include <QtWidgets/QWidget>
#include <QImage>

#include "proj.h"
#include "mapeditor.h"

class Tool;

// Main widget for editing a map via the GUI.
// Aim to keep this as thin as possible, with all the GUI-neutral stuff down in MapEditor.
class MapWidget : public QWidget, public MapEditor {
    Q_OBJECT

public:
    MapWidget() = delete;
	MapWidget(QWidget* parent, Model& model);
    virtual ~MapWidget();

    // MapEditor implementations
    virtual void CurMapChanged();
    virtual void MapModified(MapRect const& dirty);
    virtual void EntsModified();
    virtual void SetCursor(MapRect const& area);
    virtual void HideCursor();
    virtual void EntSelectionChanged();

    void ShowGrid(bool yesno);
    bool IsGridShown() const {return mShowGrid;}

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

