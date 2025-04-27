#include <QtWidgets/QtWidgets>

#include "MapSizeDialog.h"

MapSizeDialog::MapSizeDialog(QWidget *parent, int w, int h)
    : QDialog(parent)
{
    QValidator *validator = new QIntValidator(1, 65536, this);

    mWidthEdit = new QLineEdit(this);
    mWidthEdit->setText(QString::number(w));
    mWidthEdit->setValidator(validator);
    QLabel* widthlabel = new QLabel(tr("Width:"));
    widthlabel->setBuddy(mWidthEdit);

    mHeightEdit = new QLineEdit(this);
    mHeightEdit->setValidator(validator);
    mHeightEdit->setText(QString::number(h));
    QLabel* heightlabel = new QLabel(tr("Height:"));
    heightlabel->setBuddy(mHeightEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *l = new QGridLayout;
//    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    l->addWidget(widthlabel, 0, 0);
    l->addWidget(mWidthEdit, 0, 1);
    l->addWidget(heightlabel, 1, 0);
    l->addWidget(mHeightEdit, 1, 1);
    l->addWidget(buttonBox, 2, 0, 1, 2 );
    setLayout(l);
    setWindowTitle(tr("Enter map size"));
}

int MapSizeDialog::ResultW()
    {return mWidthEdit->text().toInt();}

int MapSizeDialog::ResultH()
    {return mHeightEdit->text().toInt();}

