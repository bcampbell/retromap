#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>
#include "proj.h"
#include "editor.h"

class Tool;
class MapEditor;

// View part of MVP

class MapWidget : public QWidget {
    Q_OBJECT

public:
    MapWidget() = delete;
	MapWidget(QWidget* parent);
    virtual ~MapWidget();

    // MapView methods
    void SetPresenter(MapEditor* presenter);
    void SetMap(Tilemap *tilemap, Charset *charset, Palette *palette);
    void MapModified(MapRect const& dirty);
    void SetCursor(MapRect const& area);

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

    MapRect mCursor;

    void UpdateBacking(MapRect const& dirty);
    bool IsValidMap() const {return mTilemap && mCharset && mPalette;}
};

