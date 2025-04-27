#include "PaletteWidget.h"

#include "proj.h"

#include <QPainter>
#include <QMouseEvent>

PaletteWidget::PaletteWidget(QWidget* parent, Palette& palette) :
    QWidget(parent), mPalette(palette)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

    mGridW = 8;
    mGridH = mPalette.ncolours / mGridW;

    mLeftColour = 1;
    mRightColour = 0;
}

QSize PaletteWidget::sizeHint() const
{
    return QSize(mGridW * 8 * mZoom, mGridH * 8 * mZoom);
}

void PaletteWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int picked = PickColour(event->position().toPoint());
        if (picked != -1 && picked != mLeftColour) {
            update();
            mLeftColour = picked;
            emit leftChanged(picked);
        }
    }
    if (event->button() == Qt::RightButton) {
        int picked = PickColour(event->position().toPoint());
        if (picked != -1 && picked != mRightColour) {
            update();
            mRightColour = picked;
            emit rightChanged(picked);
        }
    }
}

void PaletteWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void PaletteWidget::mouseReleaseEvent(QMouseEvent *event)
{
}

// Return the onscreen bounding box for given tile number
QRect PaletteWidget::ColourBound(int c) const
{
    QRect r = rect();
    int cw = r.width() / mGridW;
    int ch = r.height() / mGridH;
    return QRect((c % mGridW) * cw, (c / mGridW) * ch, cw, ch);
}

int PaletteWidget::PickColour(QPoint pos) const
{
    QRect r = rect();
    int cw = r.width() / mGridW;
    int ch = r.height() / mGridH;
    int cx = pos.x() / cw;
    int cy = pos.y() / ch;
    int picked = (cy * mGridW) + cx;

    if (picked < 0 || picked > mPalette.ncolours) {
        picked = -1;
    }
    return picked;
}

void PaletteWidget::paintEvent(QPaintEvent *event)
{
    QRect r = rect();
    int cw = r.width() / mGridW;
    int ch = r.height() / mGridH;

    QPainter painter(this);

    for (int cy = 0; cy < mGridH; ++cy) {
        for (int cx = 0; cx < mGridW; ++cx) {
            int i = cy*mGridW + cx;
            if (i >= mPalette.ncolours) {
                break;
            }
            uint8_t const *p = &mPalette.colours[i*4];
            QColor colour(p[0], p[1], p[2]);
            painter.fillRect(QRect(cx*cw, cy*ch, cw, ch), colour);
        }
    }

    if (mLeftColour >= 0)
    {
        QRect sel = ColourBound(mLeftColour);
        painter.setPen(QColor(255,255,255));
        painter.drawRect(sel);
    }
    if (mRightColour >= 0)
    {
        QRect sel = ColourBound(mRightColour);
        painter.setPen(QColor(128,128,128));
        painter.drawRect(sel);
    }
}


