#pragma once

#include <cstdio>
#include <vector>

#include <QtWidgets/QWidget>
#include <QImage>

#include "model.h"

class QListWidget;

// TODO: SHOULDN'T DERIVE FROM IModelListener!
// Should be IView? IEntView?
class EntWidget : public QWidget, IModelListener
{
    Q_OBJECT

public:
    EntWidget() = delete;
	EntWidget(QWidget* parent, Model& ed);
    virtual ~EntWidget();
    void SetMapNum(int mapNum);

    void SetSelection(std::vector<int> const& sel);
    std::vector<int> Selection();

    // IModelListener... (TODO - kill!)
    void ProjEntsInserted(int mapNum, int entNum, int count);
    void ProjEntsRemoved(int mapNum, int entNum, int count);
    void ProjEntChanged(int mapNum, int entNum, Ent const& oldData, Ent const& newData);
signals:
    void selectionChanged();
protected:
    void AddEnt();
    void Rebuild();
private:
    Model& mEd;
    QListWidget* mListWidget;
    int mMapNum{0};
};


