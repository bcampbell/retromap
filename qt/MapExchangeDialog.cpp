#include <QtWidgets/QtWidgets>

#include "MapExchangeDialog.h"

MapExchangeDialog::MapExchangeDialog(QWidget *parent, int map1, int map2)
    : QDialog(parent)
{
    QValidator *validator = new QIntValidator(1, 65536, this);

    mMap1Edit = new QLineEdit(this);
    mMap1Edit->setText(QString::number(map1));
    mMap1Edit->setValidator(validator);
    QLabel* fromLabel = new QLabel(tr("Exchange map 1:"));
    fromLabel->setBuddy(mMap1Edit);

    mMap2Edit = new QLineEdit(this);
    mMap2Edit->setValidator(validator);
    mMap2Edit->setText(QString::number(map2));
    QLabel* toLabel = new QLabel(tr("With map 2:"));
    toLabel->setBuddy(mMap2Edit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *l = new QGridLayout;
//    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    l->addWidget(fromLabel, 0, 0);
    l->addWidget(mMap1Edit, 0, 1);
    l->addWidget(toLabel, 1, 0);
    l->addWidget(mMap2Edit, 1, 1);
    l->addWidget(buttonBox, 2, 0, 1, 2 );
    setLayout(l);
    setWindowTitle(tr("Exchange Maps"));
}

int MapExchangeDialog::ResultMap1()
    {return mMap1Edit->text().toInt();}

int MapExchangeDialog::ResultMap2()
    {return mMap2Edit->text().toInt();}

