#include "WorldWidget.h"
#include "helpers.h"

//#include <cassert>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

constexpr int CURSORPENW = 3;

WorldWidget::WorldWidget(QWidget* parent, Model& model) : QWidget(parent), mModel(model), mCurMap(-1)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Preferred);
    setMouseTracking(true);
}

WorldWidget::~WorldWidget()
{
}


void WorldWidget::setCurMap(int mapNum)
{
    mCurMap = mapNum;
    update();
}

void WorldWidget::CalcLayout(int mapsacross)
{
    auto const& maps = mModel.proj.maps;
    mLayout.resize(mModel.proj.maps.size());

    // Calc max map size
    int maxw = std::numeric_limits<int>::min();
    int maxh = std::numeric_limits<int>::min();
    for (Tilemap const& map : maps) {
        if (map.w > maxw) {
            maxw = map.w;
        }
        if (map.h > maxh) {
            maxh = map.h;
        }
    }
    // Layout
    mExtent = MapRect();
    int x = 0;
    int y = 0;
    for (size_t i = 0; i< maps.size(); ++i) {
        mLayout[i] = MapRect(x*maxw, y*maxh, maps[i].w, maps[i].h);
        mExtent.Merge(mLayout[i]);
        ++x;
        if (x>=mapsacross) {
            x = 0;
            ++y;
        }
    }
}


int WorldWidget::PickMap(TilePoint const& p) const
{
    for (size_t i = 0; i < mLayout.size(); ++i) {
        if (mLayout[i].Contains(p)) {
            return i;
        }
    }
    return -1;
}


/*
QSize WorldWidget::sizeHint() const
{
    int w = 500;    //mCharset->tw * mTilemap->w;
    int h = 500;    //mCharset->th * mTilemap->h;
    return QSize(w, h);
}
*/

void WorldWidget::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    assert(mModel.proj.maps.size() == mOutlines.size());
    for( size_t i = 0; i< mOutlines.size(); ++i) {
        if (mOutlines[i].contains(event->position())) {
            mCurMap = (int)i;
            emit curMapChanged();
            update();
            return;
        }
    }
}

void WorldWidget::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void WorldWidget::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void WorldWidget::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void WorldWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    Proj const& proj = mModel.proj;

    CalcLayout(7);

    int tw = proj.charset.tw;
    int th = proj.charset.th;
    int zoom = 1;
    QTransform xform;
    float sx = size().width() / float(mExtent.w * tw);
    float sy = size().height() / float(mExtent.h * th);
    xform.scale(sx,sy);
    painter.setTransform(xform);

    // draw
    for (size_t i = 0; i < proj.maps.size(); ++i) {
        MapRect const& r = mLayout[i];
        QRect bound(r.x * tw * zoom, r.y * th * zoom,
            r.w * tw * zoom, r.h * th * zoom);

        /* BRUTE FORCE! */
        Tilemap const& m = proj.maps[i];
        int w = tw * m.w;
        int h = th * m.h;
        QImage img(w, h, QImage::Format_RGBX8888);

        // Draw affected area into backing image.
        for (int y = 0; y < m.h; ++y) {
            for (int x = 0; x < m.w; ++x) {
                Cell const& cell = m.CellAt(TilePoint(x, y));
                RenderCell(img, QPoint(x * tw, y * th), proj.charset, proj.palette, cell);
            }
        }

        painter.drawImage(bound,img);
    }

    // overlays
    painter.resetTransform();
    QPen unselPen(QColor(64, 64, 255), 1);
    QPen selPen(QColor(255, 255, 255), 1);

    painter.setPen(unselPen);
    painter.setBrush(Qt::NoBrush);
    for (size_t i = 0; i < proj.maps.size(); ++i) {
        MapRect const& r = mLayout[i];
        QRect bound(r.x * tw * zoom, r.y * th * zoom,
            r.w * tw * zoom, r.h * th * zoom);

        if((int)i != mCurMap) {
            painter.drawRect(mOutlines[i]);
        }
    }
    if (mCurMap >= 0) {
        painter.setPen(selPen);
        painter.drawRect(mOutlines[mCurMap]);
    }

}

void WorldWidget::resizeEvent(QResizeEvent *event)
{
    CalcLayout(7);  // TODO: magic number
    auto const& maps = mModel.proj.maps;
    mOutlines.resize(maps.size());
    int tw = mModel.proj.charset.tw;
    int th = mModel.proj.charset.th;
    float sx = size().width() / float(mExtent.w * tw);
    float sy = size().height() / float(mExtent.h * th);

    for (size_t i = 0; i < maps.size(); ++i) {
        MapRect const& r = mLayout[i];
        mOutlines[i] = QRectF((float)r.x * tw * sx, (float)r.y * th * sy, (float)r.w * tw * sx, (float)r.h * th * sy);
    }
}

