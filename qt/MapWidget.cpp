#include "MapWidget.h"
#include "helpers.h"

#include "tool.h"

//#include <cassert>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

constexpr int CURSORPENW = 3;

MapWidget::MapWidget(QWidget* parent, Model& model) : QWidget(parent), mModel(model), mPresenter(model, *this), mBacking()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Preferred);
    setMouseTracking(true);
    CurMapChanged();
}

MapWidget::~MapWidget()
{
}

void MapWidget::CurMapChanged()
{
    UpdateBacking(Map().Bounds());
    resize(sizeHint());
    update();
}

QRect MapWidget::FromMap(MapRect const& r) const {
    int tw = mModel.proj.charset.tw;
    int th = mModel.proj.charset.th;
    return QRect(r.x * tw * mZoom, r.y * th * mZoom,
        r.w * tw * mZoom, r.h * th * mZoom);
}

MapRect MapWidget::ToMap(QRectF const& r) const {
    int tw = mModel.proj.charset.tw * mZoom;
    int th = mModel.proj.charset.th * mZoom;

    MapRect out;
    out.x = r.x() / tw;
    out.y = r.y() / th;
    // We want all overlapping tiles, not just contained ones.
    out.w = (r.width() + tw - 1) / tw;
    out.h = (r.height() + th - 1) / th;
    return out;
}


void MapWidget::HideCursor()
{
    SetCursor(MapRect());
}


// TODO: cursor state should be held by presenter.
// This should be a CursorChanged notification.
void MapWidget::SetCursor(MapRect const& cursor)
{
    if (mCursor == cursor) {
        return; // no change.
    }

    const int pw = CURSORPENW;

    MapRect old = mCursor;
    mCursor = cursor;

    // Redraw old area
    if (!old.IsEmpty()) {
        QRect r = FromMap(old).adjusted(-pw,-pw, pw, pw);
        update(r);
    }

    // Redraw new area
    QRect r = FromMap(mCursor).adjusted(-pw,-pw, pw, pw);
    update(r);

    if (old != mCursor) {
        emit cursorChanged(mCursor);
    }
}

void MapWidget::MapModified(MapRect const& dirty)
{
    UpdateBacking(dirty);
    update(FromMap(dirty));
}

void MapWidget::EntsModified()
{
    // redraw all
    update();
}

void MapWidget::EntSelectionChanged()
{
    update();
}


void MapWidget::SetSelectedEnts(std::vector<int> newSelection)
{
    mPresenter.SetSelectedEnts(newSelection);
    emit entSelectionChanged();
}

void MapWidget::UpdateBacking(MapRect const& dirty)
{
    int tw = mModel.proj.charset.tw;
    int th = mModel.proj.charset.th;
    int w = tw * Map().w;
    int h = th * Map().h;
    if (mBacking.width() != w || mBacking.height() != h) {
        // resize the backing bitmap
        mBacking = QImage(w, h, QImage::Format_RGBX8888);
    }

    // Draw affected area into backing image.
    for (int y = dirty.y; y < dirty.y + dirty.h; ++y) {
        for (int x = dirty.x; x < dirty.x + dirty.w; ++x) {
            Cell const& cell = Map().CellAt(TilePoint(x, y));
            RenderCell(mBacking, QPoint(x * tw, y * th), mModel.proj.charset, mModel.proj.palette, cell);
        }
    }
}



QSize MapWidget::sizeHint() const
{
    int tw = mModel.proj.charset.tw;
    int th = mModel.proj.charset.th;
    int w = tw * Map().w;
    int h = th * Map().h;
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
    QPoint pos(event->position().toPoint());
    PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
    mPresenter.Press(this, pix, butt);
    event->accept();
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    QPoint pos(event->position().toPoint());
    PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
    mPresenter.Move(this, pix, butt);
    event->accept();
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int butt = toToolButtons(event->buttons());
    // NOTE: butt will exclude the button that was just released!
    QPoint pos(event->position().toPoint());
    PixPoint pix(pos.x() / mZoom, pos.y() / mZoom);
    mPresenter.Release(this, pix, butt);
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
        MapRect m = Map().Bounds().Clip(ToMap(event->rect()));
        if (mShowGrid) {
            // Draw grid
            QPen gridPen(QColor(0,255,0,255), 1, Qt::DotLine);
            painter.setPen(gridPen);
            int tw = mModel.proj.charset.tw * mZoom;
            int th = mModel.proj.charset.th * mZoom;
            for (int y = m.y; y <= m.y + m.h; ++y) {
                QPoint p1(m.x * tw, y * th);
                QPoint p2((m.x + m.w) *tw, y * th);
                painter.drawLine(p1, p2);
            }
            for (int x = m.x; x <= m.x + m.w; ++x) {
                QPoint p1(x * tw, m.y * th);
                QPoint p2(x * tw, (m.y + m.h) * th);
                painter.drawLine(p1, p2);
            }

            for (int y = m.y; y < m.y + m.h; ++y) {
                for (int x = m.x; x < m.x + m.w; ++x) {
                    TilePoint tp(x, y);

                    QRect bound = FromMap(MapRect(tp, 1, 1));
                    Cell const& c = Map().CellAt(tp);

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

    // Draw ents
    {
        for (int entIdx = 0; entIdx < (int)Map().ents.size(); ++entIdx) {
            Ent const& ent = Map().ents[entIdx];
            MapRect entBound = ent.Geometry();
            if (entBound.IsEmpty()) {
                continue;
            }
            bool selected = mPresenter.IsEntSelected(entIdx);

            QRect bound = FromMap(entBound);
            QPen p(QColor(0, 0, 255, selected ? 255: 128), 1);
            painter.setPen(p);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(bound);

            QPen blackPen(Qt::black,1);
            painter.setPen(blackPen);
            painter.drawRect(bound.adjusted(1,1,-1,-1));
            painter.drawRect(bound.adjusted(-1,-1,1,1));

            // draw label
            auto label = QString::fromStdString(ent.GetAttr("kind"));
            const std::vector<std::string> hidden = {"x", "y", "w", "h", "kind"};
            for (auto const& attr : ent.attrs) {
                if (std::find(hidden.begin(), hidden.end(), attr.name) == hidden.end()) {
                    label += QString::fromStdString(std::format("\n{}={}", attr.name, attr.value));
                }
            }
            painter.setPen(QColor(0,0,255,128));
            painter.drawText(bound.adjusted(2,2,0,0), Qt::AlignCenter, label);
            painter.setPen(QColor(255,255,255,128));
            painter.drawText(bound, Qt::AlignCenter, label);
        }
    }


    // Draw cursor.
    if(!mCursor.IsEmpty()) {
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

