#include "mapwidget.h"
#include "mapeditor.h"
#include "helpers.h"
#include "tool.h"

//#include <cassert>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

constexpr int CURSORPENW = 3;

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

QRect MapWidget::FromMap(MapRect const& r) const {
    int tw = mCharset->tw;
    int th = mCharset->th;
    return QRect(r.pos.x * tw * mZoom, r.pos.y * th * mZoom,
        r.w * tw * mZoom, r.h * th * mZoom);
}

MapRect MapWidget::ToMap(QRectF const& r) const {
    int tw = mCharset->tw * mZoom;
    int th = mCharset->th * mZoom;

    MapRect out;
    out.pos = TilePoint(r.x() / tw, r.y() / th);
    // We want all overlapping tiles, not just contained ones.
    out.w = (r.width() + tw - 1) / tw;
    out.h = (r.height() + th - 1) / th;
    return out;
}


void MapWidget::HideCursor()
{
    if(!mCursorOn) {
        return;
    }

    // erase existing cursor
    mCursorOn = false;
    const int pw = CURSORPENW;
    QRect r = FromMap(mCursor).adjusted(-pw,-pw, pw, pw);
    update(r);
}

void MapWidget::SetCursor(MapRect const& cursor)
{
    HideCursor();   // erase existing, if any
    mCursorOn = true;
    mCursor = cursor;
    const int pw = CURSORPENW;
    QRect r = FromMap(mCursor).adjusted(-pw,-pw, pw, pw);
    update(r);
}

void MapWidget::MapModified(MapRect const& dirty)
{
    if (IsValidMap()) {
        UpdateBacking(dirty);
        update(FromMap(dirty));
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
    if (mPresenter) {
        int butt = toToolButtons(event->buttons());
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Press(this, pix, butt);
    }
    event->accept();
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mPresenter) {
        int butt = toToolButtons(event->buttons());
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Move(this, pix, butt);
    }
    event->accept();
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mPresenter) {
        int butt = toToolButtons(event->buttons());
        // NOTE: butt will exclude the button that was just released!
        QPoint pos(event->position().toPoint());
        PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
        mPresenter->Release(this, pix, butt);
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
    // Draw the map, scaling up to our zoom level.
    {
        QRectF r = event->rect();
        QRectF src( r.x()/mZoom, r.y()/mZoom, r.width()/mZoom, r.height()/mZoom);
        painter.drawImage(r, mBacking, src);
    }

    // Draw overlays
    {
        MapRect m = mTilemap->Bounds().Clip(ToMap(event->rect()));
        if (mShowGrid) {
            // Draw grid
            QPen gridPen(QColor(0,255,0,255), 1, Qt::DotLine);
            painter.setPen(gridPen);
            int tw = mCharset->tw * mZoom;
            int th = mCharset->th * mZoom;
            for (int y = m.pos.y; y <= m.pos.y + m.h; ++y) {
                QPoint p1(m.pos.x * tw, y * th);
                QPoint p2((m.pos.x + m.w) *tw, y * th);
                painter.drawLine(p1, p2);
            }
            for (int x = m.pos.x; x <= m.pos.x + m.w; ++x) {
                QPoint p1(x * tw, m.pos.y * th);
                QPoint p2(x * tw, (m.pos.y + m.h) * th);
                painter.drawLine(p1, p2);
            }

            for (int y = m.pos.y; y < m.pos.y + m.h; ++y) {
                for (int x = m.pos.x; x < m.pos.x + m.w; ++x) {
                    TilePoint tp(x, y);

                    QRect bound = FromMap(MapRect(tp, 1, 1));
                    Cell const& c = mTilemap->CellAt(tp);

                    // text
                    QString t;
                    if (mZoom < 4) {
                       t = QString("%1").arg(c.tile);
                    } else {
                       t = QString("%1\n%2").arg(c.tile).arg(c.ink);
                    }
                    painter.setPen(QColor(0,0,0,128));
                    painter.drawText(bound.adjusted(2,2,0,0), Qt::AlignCenter, t);
                    painter.setPen(QColor(255,255,255,128));
                    painter.drawText(bound, Qt::AlignCenter, t);
                }
            }
        }
    }


    // Draw cursor.
    if(mCursorOn) {
        QRect r = FromMap(mCursor);
        QPen p(Qt::green,1);
        painter.setPen(p);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r);

        QPen blackPen(Qt::black,1);
        painter.setPen(blackPen);
        painter.drawRect(r.adjusted(1,1,-1,-1));
        painter.drawRect(r.adjusted(-1,-1,1,1));
    }

}

void MapWidget::ShowGrid(bool yesno)
{
    mShowGrid = yesno;
    update();
}

