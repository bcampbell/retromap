#include "mapwidget.h"
#include "mapeditor.h"
#include "helpers.h"
#include "tool.h"

//#include <cassert>
#include <QPainter>
#include <QMouseEvent>


MapWidget::MapWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Preferred);
    setMouseTracking(true);
}

MapWidget::~MapWidget()
{
}

void MapWidget::SetPresenter(MapEditor* presenter)
{
    mPresenter = presenter;
}

// TODO: should probably just read these from View on demand.
void MapWidget::SetMap(Tilemap* tilemap, Charset* charset, Palette* palette)
{
    mTilemap = tilemap;
    mCharset = charset;
    mPalette = palette;
    if (IsValidMap()) {
        UpdateBacking(mTilemap->Bounds());
    }
    resize(sizeHint());
    update();
}

void MapWidget::SetCursor(MapRect const& cursor)
{
    mCursor = cursor;
}

void MapWidget::MapModified(MapRect const& dirty)
{
    if (IsValidMap()) {
        UpdateBacking(dirty);
        int tw = mCharset->tw;
        int th = mCharset->th;
        update(QRect(dirty.pos.x * tw * mZoom, dirty.pos.y * th * mZoom,
            dirty.w * tw * mZoom, dirty.h * th * mZoom));
    }
}

void MapWidget::UpdateBacking(MapRect const& dirty)
{
    if (!IsValidMap()) {
        return;
    }
    int tw = mCharset->tw;
    int th = mCharset->th;
    int w = tw * mTilemap->w;
    int h = th * mTilemap->h;
    if (mBacking.width() != w || mBacking.height() != h) {
        // resize the backing bitmap
        mBacking = QImage(w, h, QImage::Format_RGBX8888);
    }

    // Draw affected area into backing image.
    for (int y = dirty.pos.y; y < dirty.pos.y + dirty.h; ++y) {
        for (int x = dirty.pos.x; x < dirty.pos.x + dirty.w; ++x) {
            Cell const& cell = mTilemap->CellAt(TilePoint(x, y));
            RenderCell(mBacking, QPoint(x * tw, y * th), *mCharset, *mPalette, cell);
        }
    }
}



QSize MapWidget::sizeHint() const
{
    if (!IsValidMap()) {
        return QSize(0,0);
    }
    int w = mCharset->tw * mTilemap->w;
    int h = mCharset->th * mTilemap->h;
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
    if (butt && mPresenter) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Press(pix, butt);
    }
    event->accept();
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    if (butt && mPresenter) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Move(pix, butt);
    }
    event->accept();
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    if (butt && mPresenter) {
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Release(pix, butt);
    }
    event->accept();
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

    // Scale the map up to our zoom level as we draw it.
    QRect src( r.x()/mZoom, r.y()/mZoom, r.width()/mZoom, r.height()/mZoom);
    painter.drawImage(r, mBacking, src);
}

