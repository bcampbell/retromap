#pragma once

#include <cstdio>
#include <vector>

#include <QtWidgets/QWidget>
#include <QImage>
#include "editor.h"


class QListWidget;

class EntWidget : public QWidget, EditListener
{
    Q_OBJECT

public:
    EntWidget() = delete;
	EntWidget(QWidget* parent, Editor& ed);
    virtual ~EntWidget();
    void SetMapNum(int mapNum);

    void SetSelection(std::vector<int> const& sel);
    std::vector<int> Selection();

    // EditListener...
    void ProjEntsInserted(int mapNum, int entNum, int count);
    void ProjEntsRemoved(int mapNum, int entNum, int count);
    void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData);
signals:
    void selectionChanged();
protected:
    void AddEnt();
    void Rebuild();
private:
    Editor& mEd;
    QListWidget* mListWidget;
    int mMapNum{0};
};


