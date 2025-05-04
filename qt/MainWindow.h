#pragma once

#include <QMainWindow>
#include <QCloseEvent>

#include "model.h"

class Model;
class CharsetWidget;
class EntWidget;
class MapWidget;
class PaletteWidget;
class PenWidget;
class WorldWidget;
class QAction;
class QLabel;


// Our main app window.
class MainWindow : public QMainWindow, IModelListener
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent, Model& ed);
    virtual ~MainWindow();

    // IModelListener
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
    void exchangeMap();
    void importMaps();
private:
    void createActions();
    void createMenus();
    bool maybeSave();

    void RethinkTitle();
    void MapNumChanged();

    // The editor state
    Model& mEd;

    MapWidget* mMapWidget;
    CharsetWidget* mCharsetWidget;
    PaletteWidget* mPaletteWidget;
    PenWidget* mPenWidget;
    WorldWidget* mWorldWidget;
    EntWidget* mEntWidget;
    QLabel* mCursorMsg;
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
       QAction* exchangeMap{nullptr};
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
       QAction* rectTool{nullptr};
       QAction* pickupTool{nullptr};
       QAction* floodFillTool{nullptr};
       QAction* entTool{nullptr};
       QAction* useCustomBrush{nullptr};
       QAction* hFlipBrush{nullptr};
       QAction* vFlipBrush{nullptr};
       QAction* drawModeNormal{nullptr};
       QAction* drawModeTile{nullptr};
       QAction* drawModeInk{nullptr};
       QAction* showGrid{nullptr};
    } mActions;

};

