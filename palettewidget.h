#pragma once

#include <QtWidgets/QWidget>
struct Palette;

class PaletteWidget : public QWidget
{
    Q_OBJECT

public:
    PaletteWidget() = delete;
	PaletteWidget(QWidget* parent, Palette& palette);

    // <0 = none selected
    int leftColour() const {return mLeftColour;}
    int rightColour() const {return mRightColour;}
    // These two don't emit 'Changed' signals.
    void setLeftColour(int idx) {mLeftColour = idx; update();}
    void setRightColour(int idx) {mRightColour = idx; update();}
signals:
    void leftChanged(int newColour);
    void rightChanged(int newColour);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;
private:
    QRect ColourBound(int c) const;
    int PickColour(QPoint pos) const;

    Palette& mPalette;
    // palette layout
    int mGridW{0};
    int mGridH{0};
    int mZoom{3};
    int mLeftColour{-1};
    int mRightColour{-1};
};


