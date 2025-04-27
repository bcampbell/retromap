#include "entwidget.h"
//#include "helpers.h"
#include "editor.h"
#include "proj.h"
#include "cmd.h"

//#include <QPainter>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>

EntWidget::EntWidget(QWidget* parent, Model& ed) :
    QWidget(parent),
    mEd(ed)
{
    mListWidget = new QListWidget(this);
    mEd.listeners.insert(this);
    QPushButton* addButton = new QPushButton("Add", this);
    QPushButton* removeButton = new QPushButton("Remove", this);
    QVBoxLayout* v = new QVBoxLayout(this);
    QHBoxLayout* h = new QHBoxLayout();

    v->addWidget(mListWidget);
    h->addWidget(addButton);
    h->addWidget(removeButton);
    v->addLayout(h);
    
 
    connect(addButton, &QPushButton::clicked, this, [&] {
            Ent e;
            e.attrs.push_back(EntAttr{"kind", "STOMPER"});
            e.attrs.push_back(EntAttr{"x", "10"});
            e.attrs.push_back(EntAttr{"y", "10"});
            e.attrs.push_back(EntAttr{"w", "4"});
            e.attrs.push_back(EntAttr{"h", "6"});

            InsertEntsCmd* cmd = new InsertEntsCmd(mEd, mMapNum, {e}, 0);
            mEd.AddCmd(cmd);
    });

    connect(removeButton, &QPushButton::clicked, this, [&] {
        auto sel = mListWidget->selectedItems();
        for (auto item : sel) {
            int row = mListWidget->row(item);
            DeleteEntsCmd* cmd = new DeleteEntsCmd(mEd, mMapNum, row, 1);
            mEd.AddCmd(cmd);
        }
    });

    connect(mListWidget, &QListWidget::itemChanged, this, [&](QListWidgetItem *item){
            int entNum = mListWidget->row(item);
            Ent newData;
            newData.FromString(item->text().toStdString());
            EditEntCmd* cmd = new EditEntCmd(mEd, mMapNum, newData, entNum);
            mEd.AddCmd(cmd);
    });
    connect(mListWidget, &QListWidget::itemSelectionChanged, this, [&](){
        emit selectionChanged();
    });

}

EntWidget::~EntWidget()
{
    mEd.listeners.erase(this);
}


void EntWidget::AddEnt()
{
}

void EntWidget::SetMapNum(int mapNum)
{
    const QSignalBlocker blocker(mListWidget);
    mMapNum = mapNum;

    mListWidget->clear();

    Tilemap& map = mEd.GetMap(mMapNum);
    for (Ent const& ent : map.ents) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QString::fromStdString(ent.ToString()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        mListWidget->addItem(item);
    }
}

void EntWidget::ProjEntsInserted(int mapNum, int entNum, int count)
{
    if (mapNum != mMapNum) {
        return; // Not our problem.
    }
    const QSignalBlocker blocker(mListWidget);
    for (int i = 0; i < count; ++i) {
        Ent const& ent = mEd.GetEnt(mMapNum, entNum + i);
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QString::fromStdString(ent.ToString()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        mListWidget->insertItem(entNum + i, item);
    }
}

void EntWidget::ProjEntsRemoved(int mapNum, int entNum, int count)
{
    if (mapNum != mMapNum) {
        return; // Not our problem.
    }
    const QSignalBlocker blocker(mListWidget);
    for (int i = 0; i < count; ++i) {
        delete mListWidget->item(entNum);
    }
}

void EntWidget::ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData)
{
    if (mapNum != mMapNum) {
        return; // Not our problem.
    }
    const QSignalBlocker blocker(mListWidget);
    Ent const& ent = mEd.GetEnt(mapNum, entNum);
    QListWidgetItem* item = mListWidget->item(entNum);
    item->setText(QString::fromStdString(ent.ToString()));
}

void EntWidget::SetSelection(std::vector<int> const& sel)
{
    const QSignalBlocker blocker(mListWidget);
    for (int i = 0; i < mListWidget->count(); ++i) {
        auto item = mListWidget->item(i);
        if (std::find(sel.begin(), sel.end(), i) == sel.end()) {
            item->setSelected(false);
        } else {
            item->setSelected(true);
        }
    }
}

std::vector<int> EntWidget::Selection() {

    std::vector<int> sel;
    auto l = mListWidget->selectedItems();
    for (auto item : l) {
        sel.push_back(mListWidget->row(item));
    }
    return sel;
}

