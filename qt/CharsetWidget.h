#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>
#include "proj.h"

class CharsetWidget : public QWidget
{
    Q_OBJECT

public:
    CharsetWidget() = delete;
	CharsetWidget(QWidget* parent);

    void SetTiles(Charset* tiles, Palette* palette);

    // <0 = none selected
    int leftTile() const {return mLeftTile;}
    int rightTile() const {return mRightTile;}
    // These don't emit 'changed' signals.
    void setLeftTile(int tile) {mLeftTile = tile; update();}
    void setRightTile(int tile) {mRightTile = tile; update();}
signals:
    void leftChanged(int newTile);
    void rightChanged(int newTile);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
//    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
//    void resizeEvent(QResizeEvent *event);
    QSize sizeHint() const override;
private:
    QRect TileBound(int tile) const;
    void InitTiles();
    int PickTile(QPoint pos) const;

    Charset* mTiles{nullptr};
    Palette* mPalette{nullptr};

    // tile layout
    int mGridW{0};
    int mGridH{0};
    int mZoom{2};
    QImage mBacking;
    int mLeftTile{-1};
    int mRightTile{-1};
};


