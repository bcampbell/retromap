#include "penwidget.h"
#include "helpers.h"
#include "editor.h"
#include "proj.h"

#include <QPainter>
#include <QMouseEvent>

PenWidget::PenWidget(QWidget* parent, Editor& ed) :
    QWidget(parent),
    mEd(ed)
{
    setMinimumSize( 64,32);
    mEd.listeners.insert(this);
}

PenWidget::~PenWidget() {
    mEd.listeners.erase(this);
}

void PenWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    Proj& proj = mEd.proj;
    int tw = proj.charset.tw;
    int th = proj.charset.th;
    QImage img(tw,th, QImage::Format_RGBX8888);
    RenderCell(img, QPoint(0, 0), proj.charset, proj.palette, mEd.leftPen);

    QRect out(4, 4, tw*mZoom, th*mZoom);
    painter.drawImage(out, img);
    RenderCell(img, QPoint(0, 0), proj.charset, proj.palette, mEd.rightPen);
    out.translate(4 + out.width(), 0);
    painter.drawImage(out, img);
}


