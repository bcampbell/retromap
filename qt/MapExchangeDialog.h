#pragma once

#include <QtWidgets/QDialog>

class QLineEdit;

class MapExchangeDialog : public QDialog
{
    Q_OBJECT

public:
    MapExchangeDialog(QWidget *parent, int map1, int map2);

    int ResultMap1();
    int ResultMap2();
private:
    QLineEdit *mMap1Edit;
    QLineEdit *mMap2Edit;
};

