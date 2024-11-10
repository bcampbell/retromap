#include "charsetwidget.h"
#include "helpers.h"

#include <QPainter>
#include <QMouseEvent>

CharsetWidget::CharsetWidget(QWidget* parent) :
    QWidget(parent),
    mTiles(nullptr),
    mPalette(nullptr)
{
//    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mLeftTile = 0;
    mRightTile = 32;
}

void CharsetWidget::SetTiles(Charset* tiles, Palette* palette)
{
    mTiles = tiles;
    mPalette = palette;
    InitTiles();
    resize(sizeHint());
    update();
}



QSize CharsetWidget::sizeHint() const
{
    int tw = mTiles ? mTiles->tw : 8;
    int th = mTiles ? mTiles->th : 8;

    return QSize(mGridW * tw * mZoom, mGridH * th * mZoom);
}


void CharsetWidget::InitTiles()
{
    if (!mTiles) {
        return;
    }
    mGridW = 16;
    mGridH = mTiles->ntiles / mGridW;

    // Render charset to backing image
    int tw = mTiles->tw;
    int th = mTiles->th;
    mBacking = QImage(mGridW * tw, mGridH * th, QImage::Format_RGBX8888);
    Cell cell;
    cell.tile = 0;
    cell.ink = 5;
    cell.paper = 0;
    for (int ty = 0; ty < mGridH; ++ty) {
        for (int tx = 0; tx < mGridW; ++tx) {
            RenderCell(mBacking, QPoint(tx * tw, ty * th), *mTiles, *mPalette, cell);
            cell.tile++;
            if (cell.tile >= mTiles->ntiles) {
                break;
            }
        }
    }
};

void CharsetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int picked = PickTile(event->position().toPoint());
        if (picked != -1 && picked != mLeftTile) {
            update();
            /*
            update(TileBound(mLeftTile));
            update(TileBound(picked));
            */

            mLeftTile = picked;
            emit leftChanged(picked);
        }
    }
    if (event->button() == Qt::RightButton) {
        int picked = PickTile(event->position().toPoint());
        if (picked != -1 && picked != mRightTile) {
            update();
//            update(TileBound(mRightTile));
//            update(TileBound(picked));

            mRightTile = picked;
            emit rightChanged(picked);
        }
    }
}

void CharsetWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void CharsetWidget::mouseReleaseEvent(QMouseEvent *event)
{
}

// Return the onscreen bounding box for given tile number
QRect CharsetWidget::TileBound(int tile) const
{
    int tw = mTiles ? mTiles->tw : 8;
    int th = mTiles ? mTiles->th : 8;
    return QRect((tile % mGridW) * tw * mZoom,
        (tile / mGridW) * th * mZoom,
        tw * mZoom,
        th * mZoom);
}

// Return tile under pos.
int CharsetWidget::PickTile(QPoint pos) const
{
    if (!mTiles) {
        return -1;
    }
    int tw = mTiles->tw;
    int th = mTiles->th;
    int tx = pos.x() / (tw * mZoom);
    int ty = pos.y() / (th * mZoom);
    int picked = (ty * mGridW) + tx;

    if (picked < 0 || picked > mTiles->ntiles) {
        picked = -1;
    }
    return picked;
}

void CharsetWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect r = event->rect();
    //printf("paint %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
    QRect src( r.x()/mZoom, r.y()/mZoom, r.width()/mZoom, r.height()/mZoom);
    painter.drawImage(r, mBacking, src);

    {
        QRect sel = TileBound(mLeftTile);
        painter.setPen(QColor(255,255,255));
        painter.drawRect(sel);
    }
    {
        QRect sel = TileBound(mRightTile);
        painter.setPen(QColor(128,128,128));
        painter.drawRect(sel);
    }
}


