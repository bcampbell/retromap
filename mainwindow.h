#pragma once

#include <QMainWindow>
#include <QCloseEvent>

#include "proj.h"
#include "editor.h"
#include "mapeditor.h"

class MapWidget;
class PaletteWidget;
class PenWidget;
class CharsetWidget;
class QAction;


// Map editing window.
class MainWindow : public QMainWindow, EditListener
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent, Editor& ed);
    virtual ~MainWindow();

    // EditListener
    virtual void EditorPenChanged();
    virtual void EditorBrushChanged();
    virtual void EditorToolChanged();
    virtual void ProjCharsetModified();
    virtual void ProjMapModified(int mapNum, MapRect const& dirty);
    virtual void ProjMapsInserted(int mapNum, int count);
    virtual void ProjMapsRemoved(int mapNum, int count);
protected:
    void closeEvent(QCloseEvent *event) override;
    void createWidgets();
private slots:
    void importCharset();
    void open();
    bool save();
    bool saveAs();
    void help();
    void addMap();
    void resizeMap();
    void importMaps();
private:
    void createActions();
    void createMenus();
    bool maybeSave();

    void RethinkTitle();

    // The editor state
    Editor& mEd;

    MapWidget* mMapWidget;
    MapEditor  mPresenter;
    CharsetWidget* mCharsetWidget;
    PaletteWidget* mPaletteWidget;
    PenWidget* mPenWidget;
    struct {
       QAction* importCharset{nullptr};
       QAction* open{nullptr};
       QAction* save{nullptr};
       QAction* saveAs{nullptr};
       QAction* exit{nullptr};
       QAction* help{nullptr};
       QAction* addMap{nullptr};
       QAction* deleteMap{nullptr};
       QAction* resizeMap{nullptr};
       QAction* importMaps{nullptr};
       QAction* mapNext{nullptr};
       QAction* mapPrev{nullptr};
       QAction* mapNorth{nullptr};
       QAction* mapSouth{nullptr};
       QAction* mapWest{nullptr};
       QAction* mapEast{nullptr};
       QAction* undo{nullptr};
       QAction* redo{nullptr};
       QAction* drawTool{nullptr};
       QAction* pickupTool{nullptr};
       QAction* rectTool{nullptr};
       QAction* useCustomBrush{nullptr};
       QAction* drawModeNormal{nullptr};
       QAction* drawModeTile{nullptr};
       QAction* drawModeInk{nullptr};
    } mActions;

};

