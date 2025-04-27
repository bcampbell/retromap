#pragma once

#include <cstdio>

#include <QtWidgets/QWidget>
#include <QImage>
#include "editor.h"

class Model;

// TODO: not IModelListener!
class PenWidget : public QWidget, IModelListener
{
    Q_OBJECT

public:
    PenWidget() = delete;
	PenWidget(QWidget* parent, Model& ed);
    virtual ~PenWidget();

    // IModelListener... (TODO: nope!)
    void EditorPenChanged() {update();}
protected:
    void paintEvent(QPaintEvent *event);
//    void resizeEvent(QResizeEvent *event);
private:
    Model& mEd;
    int mZoom{4};
};


