#include "mapwidget.h"
#include "helpers.h"
#include "tool.h"

//#include <cassert>
#include <QPainter>
#include <QMouseEvent>


MapWidget::MapWidget(QWidget* parent, Editor& ed) :
    QWidget(parent),
    mEd(ed),
    mProj(ed.proj),
    mBacking(),
    mTool(new DrawTool(ed))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Preferred);
    setMouseTracking(true);
    mEd.listeners.insert(this);
    SetCurrentMap(0);
}

MapWidget::~MapWidget()
{
    mEd.listeners.erase(this);
    delete mTool;
}

void MapWidget::SetCurrentMap(int mapNum) {
    assert(mapNum >= 0 && mapNum < (int)mProj.maps.size());
    if (mCurMap != mapNum || mBacking.isNull()) {
        mCurMap = mapNum;

        // resize the backing bitmap
        Tilemap& map = mProj.maps[mapNum];
        int w = mProj.charset.tw * map.w;
        int h = mProj.charset.th * map.h;
        mBacking = QImage(w, h, QImage::Format_RGBX8888),
        resize(sizeHint());
        // force redraw
        ProjMapModified(mCurMap, map.Bounds());
    }
}


QSize MapWidget::sizeHint() const
{
    Tilemap& map = mProj.maps[mCurMap];
    int w = mProj.charset.tw * map.w;
    int h = mProj.charset.th * map.h;
    return QSize(w * mZoom, h * mZoom);
}

static int toToolButtons(Qt::MouseButtons qb)
{
    int out = 0;
    if (qb & Qt::LeftButton) {
        out |= Tool::LEFT;
    }
    if (qb & Qt::RightButton) {
        out |= Tool::RIGHT;
    }
    return out;
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    if (butt) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mTool->Press(mCurMap, pix, butt);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    if (butt) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mTool->Move(mCurMap, pix, butt);
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    if (butt) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mTool->Release(mCurMap, pix, butt);
    }
}

void MapWidget::wheelEvent(QWheelEvent *event)
{

    if (event->angleDelta().y() < 0) {
        if (mZoom >1) {
            --mZoom;
            resize(sizeHint());
            update();
        }
    } else if (event->angleDelta().y() > 0) {
        if (mZoom < 8) {
            ++mZoom;
            resize(sizeHint());
            update();
        }
    }
    event->accept();
}


void MapWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect r = event->rect();
    //painter.drawImage(QPoint(0,0), mBacking);
    //printf("paint %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());

    //QRect t(r.x(), r.y(), r.width()*4, r.height()*4);
    QRect src( r.x()/mZoom, r.y()/mZoom, r.width()/mZoom, r.height()/mZoom);
    painter.drawImage(r, mBacking, src);
}

// EditListener
void MapWidget::ProjMapModified(int mapNum, MapRect const& dirty)
{
    if (mCurMap != mapNum) {
        return; // Happened on some other map.
    }
    Tilemap& map = mProj.maps[mCurMap];
    int tw = mProj.charset.tw;
    int th = mProj.charset.th;
    // update the backing image
    for (int y = dirty.pos.y; y < dirty.pos.y + dirty.h; ++y) {
        for (int x = dirty.pos.x; x < dirty.pos.x + dirty.w; ++x) {
            Cell const& cell = map.CellAt(TilePoint(x, y));
            RenderCell(mBacking, QPoint(x * tw, y * th), mProj.charset, mProj.palette, cell);
        }
    }

    // redraw affected area
    update(QRect(dirty.pos.x * tw * mZoom, dirty.pos.y * th * mZoom,
            dirty.w * tw * mZoom, dirty.h * th * mZoom));
}


// EditListener
void MapWidget::ProjMapsInserted(int first, int count)
{
    if (mCurMap >= first) {
        SetCurrentMap(mCurMap + count);
    }
}

void MapWidget::ProjMapsRemoved(int first, int count)
{
    if (mCurMap >= first+count) {
        SetCurrentMap(mCurMap - count);
        return;
    }
    if (mCurMap >= first) {
        SetCurrentMap(std::min(int(mProj.maps.size()) - 1, first));
    }
}

void MapWidget::ProjCharsetModified()
{
    Tilemap& map = mProj.maps[mCurMap];
    ProjMapModified(mCurMap, map.Bounds());
}

