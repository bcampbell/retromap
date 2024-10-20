#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>
#include "editor.h"

class Editor;

class PenWidget : public QWidget, EditListener
{
    Q_OBJECT

public:
    PenWidget() = delete;
	PenWidget(QWidget* parent, Editor& ed);
    virtual ~PenWidget();

    // EditListener...
    void EditorPenChanged() {update();}
protected:
    void paintEvent(QPaintEvent *event);
//    void resizeEvent(QResizeEvent *event);
private:
    Editor& mEd;
    int mZoom{4};
};


