#pragma once

#include <QMainWindow>
#include <QCloseEvent>

#include "proj.h"
#include "editor.h"

class MapWidget;
class PaletteWidget;
class PenWidget;
class TilesetWidget;
class QAction;


// Map editing window.
class MainWindow : public QMainWindow, EditListener
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent, Editor& ed);
    virtual ~MainWindow();

    // EditListener
    virtual void ProjPenChanged();
    virtual void ProjTilesetModified();
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjMapsInserted(int mapNum, int count);
    virtual void ProjMapsRemoved(int mapNum, int count);
protected:
    void closeEvent(QCloseEvent *event) override;
    void createWidgets();
private slots:
    void importTileset();
    void open();
    bool save();
    bool saveAs();
    void help();
    void addMap();
    void nextMap();
    void prevMap();
private:
    void createActions();
    void createMenus();
    bool maybeSave();

    // The editor state
    Editor& mEd;

    MapWidget* mMapWidget;
    TilesetWidget* mTilesetWidget;
    PaletteWidget* mPaletteWidget;
    PenWidget* mPenWidget;
    struct {
       QAction* importTileset{nullptr};
       QAction* open{nullptr};
       QAction* save{nullptr};
       QAction* saveAs{nullptr};
       QAction* exit{nullptr};
       QAction* help{nullptr};
       QAction* addMap{nullptr};
       QAction* nextMap{nullptr};
       QAction* prevMap{nullptr};
       QAction* undo{nullptr};
       QAction* redo{nullptr};
    } mActions;

};

