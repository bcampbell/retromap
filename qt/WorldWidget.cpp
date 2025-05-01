#include "WorldWidget.h"
#include "helpers.h"

//#include <cassert>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

constexpr int CURSORPENW = 3;

WorldWidget::WorldWidget(QWidget* parent, Model& model) : QWidget(parent), mModel(model)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Preferred);
    setMouseTracking(true);
}

WorldWidget::~WorldWidget()
{
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

    int worldw = 7;
    int worldh = 7;

    //
    std::vector<Rect> layout(proj.maps.size());
    // Calc max map size
    int maxw = std::numeric_limits<int>::min();
    int maxh = std::numeric_limits<int>::min();
    for (Tilemap const& map : proj.maps) {
        if (map.w > maxw) {
            maxw = map.w;
        }
        if (map.h > maxh) {
            maxh = map.h;
        }
    }
    // Layout
    size_t i=0;
    for (int wy = 0; wy < worldh; ++wy) {
        for (int wx = 0; wx < worldw; ++wx) {
            if (i<proj.maps.size()) {
                layout[i] = Rect(wx*maxw, wy*maxh, proj.maps[i].w, proj.maps[i].h);
            }
            ++i;
        }
    }

    int tw = proj.charset.tw;
    int th = proj.charset.th;
    int zoom = 1;
    QRectF logical(0,0,(tw*worldw*maxw), (th*worldh*maxh));
    QTransform xform;
    float sx = size().width() / logical.width();
    float sy = size().height() / logical.height();
    xform.scale(sx,sy);
    painter.setTransform(xform);

    // draw
    QPen p(QColor(0, 0, 255), 1);
    painter.setPen(p);
    painter.setBrush(Qt::NoBrush);
    for (size_t i = 0; i < proj.maps.size(); ++i) {
        Rect const& r = layout[i];
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

        painter.drawRect(bound);
    }

}

void WorldWidget::resizeEvent(QResizeEvent *event)
{
}

