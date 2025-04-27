#pragma once

#include <QtWidgets/QDialog>

class QLineEdit;

class MapSizeDialog : public QDialog
{
    Q_OBJECT

public:
    MapSizeDialog(QWidget *parent, int w, int h);

    int ResultW();
    int ResultH();
private:
    QLineEdit *mWidthEdit;
    QLineEdit *mHeightEdit;
};

