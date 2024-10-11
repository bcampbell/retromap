#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>
#include "proj.h"
#include "editor.h"

class Tool;

class MapWidget : public QWidget, EditListener {
    Q_OBJECT

public:
    MapWidget() = delete;
	MapWidget(QWidget* parent, Editor& ed);
    virtual ~MapWidget();

    // set which map we're looking at
    void SetCurrentMap(int mapNum);
    int CurrentMap() {return mCurMap;}
    // EditListener
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;

private:
    Editor& mEd;
    Proj& mProj;
    QImage mBacking;
    int mZoom{3};
    Tool* mTool;
    int mCurMap{0};

    PixPoint ToPixPoint(QPoint const& pt) const
        {return PixPoint(pt.x()/mZoom, pt.y()/mZoom);}
    void PlonkTile(int x, int y, Cell const& pen);

};

