#include "MainWindow.h"

#include "CharsetWidget.h"
#include "EntWidget.h"
#include "helpers.h"
#include "MapWidget.h"
#include "MapSizeDialog.h"
#include "MapExchangeDialog.h"
#include "PaletteWidget.h"
#include "PenWidget.h"

#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "cmd.h"
#include "draw.h"
#include "model.h"

MainWindow::MainWindow(QWidget *parent, Model& ed)
    : QMainWindow(parent), mEd(ed), mPresenter(ed)
{
    mEd.modified = false;

    mEd.listeners.insert(this);
    createActions();

    switch (mEd.drawFlags) {
        case DRAWFLAG_ALL: mActions.drawModeNormal->setChecked(true); break;
        case DRAWFLAG_TILE: mActions.drawModeTile->setChecked(true); break;
        case DRAWFLAG_INK: mActions.drawModeInk->setChecked(true); break;
    }

    createMenus();
    createWidgets();
    RethinkTitle();
    resize(500, 500);

    mPresenter.AddView(mMapWidget);
    show();
}

MainWindow::~MainWindow() {
    mPresenter.RemoveView(mMapWidget);
    mEd.listeners.erase(this);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::RethinkTitle()
{
    QString t = QString("Retromap map %1/%2 - %3")
        .arg(mPresenter.CurrentMap())
        .arg((int)mEd.proj.maps.size())
        .arg(QString::fromStdString(mEd.mapFilename));
    setWindowTitle(t);
}


// 
void MainWindow::MapNumChanged()
{
    mEntWidget->SetMapNum(mPresenter.CurrentMap());
    RethinkTitle();
}

bool MainWindow::maybeSave()
{
    if (mEd.modified) {
       QMessageBox::StandardButton ret;
       ret = QMessageBox::warning(this, tr("RetroMap"),
                          tr("The map has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard
                          | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::createActions()
{
    mActions.importCharset = new QAction(tr("&Import charset..."), this);
    connect(mActions.importCharset, &QAction::triggered, this, &MainWindow::importCharset);

    {
        QAction* a;
        a = new QAction(tr("&Undo"), this);
        a->setShortcuts(QKeySequence::Undo);
        connect(a, &QAction::triggered, [&]() {mEd.Undo(); RethinkTitle();});
        mActions.undo = a;

        a = new QAction(tr("&Redo"), this);
        a->setShortcuts(QKeySequence::Redo);
        connect(a, &QAction::triggered, [&]() {mEd.Redo(); RethinkTitle();});
        mActions.redo = a;
    }

    mActions.open = new QAction(tr("&Open map..."), this);
    mActions.open->setShortcuts(QKeySequence::Open);
    connect(mActions.open, &QAction::triggered, this, &MainWindow::open);

    mActions.save = new QAction(tr("&Save map"), this);
    mActions.save->setShortcuts(QKeySequence::Save);
    connect(mActions.save, &QAction::triggered, this, &MainWindow::save);

    mActions.saveAs = new QAction(tr("&Save map as..."), this);
    mActions.saveAs->setShortcuts(QKeySequence::SaveAs);
    connect(mActions.saveAs, &QAction::triggered, this, &MainWindow::saveAs);

    mActions.exit = new QAction(tr("E&xit"), this);
    mActions.exit->setShortcuts(QKeySequence::Quit);
    connect(mActions.exit, &QAction::triggered, this, &MainWindow::close);

    mActions.help = new QAction(tr("Help"), this);
    mActions.help->setShortcuts(QKeySequence::HelpContents);
    connect(mActions.help, &QAction::triggered, this, &MainWindow::help);

    // Add map
    {
        QAction* a = new QAction(tr("Add map"), this);
        a->setShortcut(QKeySequence(Qt::Key_A));
        connect(a, &QAction::triggered, this, &MainWindow::addMap);
        mActions.addMap = a;
    }
    // Delete current map
    {
        QAction* a = new QAction(tr("Delete map"), this);
        connect(a, &QAction::triggered, [&](){
             // Can't delete last map.
            if (mEd.proj.maps.size() > 1) {
                int cur = mPresenter.CurrentMap();
                auto* cmd = new DeleteMapsCmd(mEd, cur, cur + 1);
                mEd.AddCmd(cmd);
            }
        });
        mActions.deleteMap = a;
    }
    // Exchange map
    {
        QAction* a= new QAction(tr("Exchange map..."), this);
        connect(a, &QAction::triggered, this, &MainWindow::exchangeMap);
        mActions.exchangeMap = a;
    }
    // Resize current map
    {
        QAction* a= new QAction(tr("Resize map..."), this);
        connect(a, &QAction::triggered, this, &MainWindow::resizeMap);
        mActions.resizeMap = a;
    }
    // Import maps
    {
        QAction* a= new QAction(tr("Import maps..."), this);
        connect(a, &QAction::triggered, this, &MainWindow::importMaps);
        mActions.importMaps = a;
    }


    // navigate around maps
    {
        QAction* a;
        a = new QAction(tr("North"), this);
        a->setShortcut(QKeySequence(Qt::Key_Up));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(0, -1);
            MapNumChanged();
        });
        mActions.mapNorth = a;
        
        a = new QAction(tr("South"), this);
        a->setShortcut(QKeySequence(Qt::Key_Down));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(0, 1);
            MapNumChanged();
        });
        mActions.mapSouth = a;

        a = new QAction(tr("West"), this);
        a->setShortcut(QKeySequence(Qt::Key_Left));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(-1, 0);
            MapNumChanged();
        });
        mActions.mapWest = a;

        a = new QAction(tr("East"), this);
        a->setShortcut(QKeySequence(Qt::Key_Right));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(1, 0);
            MapNumChanged();
        });
        mActions.mapEast = a;

        a = new QAction(tr("Next"), this);
        a->setShortcut(QKeySequence(Qt::Key_Plus));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNavLinear(1);
            MapNumChanged();
        });
        mActions.mapNext = a;

        a = new QAction(tr("Previous"), this);
        a->setShortcut(QKeySequence(Qt::Key_Minus));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNavLinear(-1);
            MapNumChanged();
        });
        mActions.mapPrev = a;
    }    

    // Tools

    {
        QActionGroup *toolGroup = new QActionGroup(this);
        QAction* a;

        mActions.drawTool = a = new QAction(tr("Draw"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_D));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_DRAW);
        });

        mActions.rectTool = a = new QAction(tr("Rectangle"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_R));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_RECT);
        });

        mActions.pickupTool = a = new QAction(tr("Pick up brush"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_B));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_PICKUP);
        });

        mActions.floodFillTool = a = new QAction(tr("Flood fill"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_F));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_FLOODFILL);
        });

        mActions.entTool = a = new QAction(tr("Ent Placement"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_E));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_ENT);
        });
    }

    {
        QActionGroup *brushGroup = new QActionGroup(this);
        QAction* a;

        mActions.useCustomBrush = a = new QAction(tr("Use Brush"), brushGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_Period));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.useBrush = !self->mEd.useBrush;
            for (auto l : self->mEd.listeners) {
                l->EditorBrushChanged();
            }
        });
    }

    //
    {
        QAction* a;
        mActions.hFlipBrush = a = new QAction(tr("Flip Brush Horizontally"));
        a->setShortcut(QKeySequence(Qt::Key_X));
        connect(a, &QAction::triggered, this, [&]() {
            if (mEd.brush.Bounds().IsEmpty()) {
                return;
            }
            HFlip(mEd.brush);
            for (auto l : mEd.listeners) {
                l->EditorBrushChanged();
            }
        });

        mActions.vFlipBrush = a = new QAction(tr("Flip Brush Vertically"));
        a->setShortcut(QKeySequence(Qt::Key_Y));
        connect(a, &QAction::triggered, this, [&]() {
            if (mEd.brush.Bounds().IsEmpty()) {
                return;
            }
            VFlip(mEd.brush);
            for (auto l : mEd.listeners) {
                l->EditorBrushChanged();
            }
        });
    }


    // Draw modes

    {
        QActionGroup *drawModeGroup = new QActionGroup(this);
        QAction* a;

        mActions.drawModeNormal = a = new QAction(tr("Normal"), drawModeGroup);
        a->setCheckable(true);
        //a->setShortcut(QKeySequence(Qt::Key_D));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.drawFlags = DRAWFLAG_ALL;
        });

        mActions.drawModeTile = a = new QAction(tr("Tile only"), drawModeGroup);
        a->setCheckable(true);
        //a->setShortcut(QKeySequence(Qt::Key_B));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.drawFlags = DRAWFLAG_TILE;
        });

        mActions.drawModeInk = a = new QAction(tr("Ink only"), drawModeGroup);
        a->setCheckable(true);
        //a->setShortcut(QKeySequence(Qt::Key_B));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.drawFlags = DRAWFLAG_INK;
        });
    }


    // Show grid
    {
        QAction* a = new QAction(tr("Show grid"));
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_G));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mMapWidget->ShowGrid(!self->mMapWidget->IsGridShown());
        });
        mActions.showGrid = a;
    }
}


void MainWindow::createMenus()
{
    {
        QMenu* m = new QMenu(tr("&File"), this);
        m->addAction(mActions.importCharset);
        m->addAction(mActions.open);
        m->addAction(mActions.importMaps);
        m->addSeparator();
        m->addAction(mActions.save);
        m->addAction(mActions.saveAs);
        m->addSeparator();
        m->addAction(mActions.exit);
        menuBar()->addMenu(m);
    }
    {
        QMenu* m = new QMenu(tr("&Edit"), this);
        m->addAction(mActions.undo);
        m->addAction(mActions.redo);
        m->addSeparator();
        m->addAction(mActions.drawTool);
        m->addAction(mActions.rectTool);
        m->addAction(mActions.pickupTool);
        m->addAction(mActions.floodFillTool);
        m->addAction(mActions.entTool);
        m->addSeparator();
        m->addAction(mActions.useCustomBrush);
        m->addAction(mActions.hFlipBrush);
        m->addAction(mActions.vFlipBrush);
        menuBar()->addMenu(m);
    }
    {
        QMenu* m = new QMenu(tr("&Map"), this);
        m->addAction(mActions.mapNext);
        m->addAction(mActions.mapPrev);
        m->addAction(mActions.mapNorth);
        m->addAction(mActions.mapSouth);
        m->addAction(mActions.mapWest);
        m->addAction(mActions.mapEast);
        m->addSeparator();
        m->addAction(mActions.addMap);
        m->addAction(mActions.deleteMap);
        m->addAction(mActions.resizeMap);
        m->addAction(mActions.exchangeMap);
        m->addSeparator();
        m->addAction(mActions.showGrid);
        menuBar()->addMenu(m);
    }
    {
        QMenu* m = new QMenu(tr("&Drawmode"), this);
        m->addAction(mActions.drawModeNormal);
        m->addAction(mActions.drawModeTile);
        m->addAction(mActions.drawModeInk);
        menuBar()->addMenu(m);
    }
    {
        QMenu* m = new QMenu(tr("&Help"), this);
        m->addAction(mActions.help);
        menuBar()->addMenu(m);
    }
}


void MainWindow::createWidgets()
{
    // Status bar
    // Add a label for displaying the cursor status.
    mCursorMsg = new QLabel();
    statusBar()->addWidget(mCursorMsg);

    // The main map editing area
    mMapWidget = new MapWidget(nullptr);

    mCharsetWidget = new CharsetWidget(nullptr);

    mCharsetWidget->SetTiles(&mEd.proj.charset, &mEd.proj.palette);

    mCharsetWidget->setLeftTile(mEd.leftPen.tile);
    mCharsetWidget->setRightTile(mEd.rightPen.tile);

    mPenWidget = new PenWidget(nullptr, mEd);

    mPaletteWidget = new PaletteWidget(nullptr, mEd.proj.palette);
    mPaletteWidget->setLeftColour(mEd.leftPen.ink);
    mPaletteWidget->setRightColour(mEd.rightPen.ink);


    // Set up Ent widget
    mEntWidget = new EntWidget(this, mEd);
    mEntWidget->SetMapNum(0);

    // Stupid QMainWindow requires a single central widget
    QWidget *central = new QWidget();

    QHBoxLayout *top = new QHBoxLayout(central);
    {
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setWidget(mMapWidget);
        scroll->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        top->addWidget(scroll, 100);
    }

    QVBoxLayout *h = new QVBoxLayout();
    {
        QVBoxLayout *v = new QVBoxLayout();

        QToolBar *toolbar = new QToolBar(this);
        toolbar->addAction(mActions.drawTool);
        toolbar->addAction(mActions.rectTool);
        toolbar->addAction(mActions.pickupTool);
        toolbar->addAction(mActions.floodFillTool);
        toolbar->addAction(mActions.entTool);
        v->addWidget(toolbar, Qt::AlignLeading);

        QToolBar *drawModeBar = new QToolBar(this);
        drawModeBar->addAction(mActions.drawModeNormal);
        drawModeBar->addAction(mActions.drawModeTile);
        drawModeBar->addAction(mActions.drawModeInk);
        v->addWidget(drawModeBar, Qt::AlignLeading);

        v->addWidget(mPenWidget, Qt::AlignLeading);
        v->addWidget(mPaletteWidget, Qt::AlignLeading);
        h->addLayout(v);
    }
    {
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setWidget(mCharsetWidget);
        h->addWidget(scroll, Qt::AlignLeading);
    }
    h->addWidget(mEntWidget, Qt::AlignLeading);

    top->addLayout(h);
    setCentralWidget(central);


    // Wire it all up!

    connect(mMapWidget, &MapWidget::cursorChanged, this, [&](MapRect const& cursor) {
        // Show the cursor position on the status bar.
        QString msg;
        if(!cursor.IsEmpty()) {
            msg = QString("%1,%2 (%3x%4)").arg(cursor.x).arg(cursor.y).arg(cursor.w).arg(cursor.h);
        }
        mCursorMsg->setText(msg);
    });

    connect(mMapWidget, &MapWidget::entSelectionChanged, this, [&]() {
        mEntWidget->SetSelection(mPresenter.SelectedEnts());
    });

    connect(mEntWidget, &EntWidget::selectionChanged, this, [&]() {
       auto sel = mEntWidget->Selection();
       mPresenter.SetSelectedEnts(sel);
    });


    connect(mCharsetWidget, &CharsetWidget::leftChanged, this, [self=this](int tile) {
        self->mEd.leftPen.tile = tile;
        for (auto l : self->mEd.listeners) {
            l->EditorPenChanged();
        }
    });

    connect(mCharsetWidget, &CharsetWidget::rightChanged, this, [self=this](int tile) {
        self->mEd.rightPen.tile = tile;
        for (auto l : self->mEd.listeners) {
            l->EditorPenChanged();
        }
    });


    connect(mPaletteWidget, &PaletteWidget::leftChanged, this, [self=this](int ink){
        self->mEd.leftPen.ink = ink;
        for (auto l : self->mEd.listeners) {
            l->EditorPenChanged();
        }
    });
    connect(mPaletteWidget, &PaletteWidget::rightChanged, this, [self=this](int ink){
        self->mEd.rightPen.ink = ink;
        for (auto l : self->mEd.listeners) {
            l->EditorPenChanged();
        }
    });
}


bool MainWindow::save()
{
    if (mEd.mapFilename.empty()) {
        return saveAs();
    }

    bool ok = SaveProject(mEd.proj, QString::fromStdString(mEd.mapFilename));
    if (!ok) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Save failed"), tr("Poop. It's all gone pear-shaped."));
        return false;
    }

    mEd.modified = false;
    return true;
}

bool MainWindow::saveAs()
{
    /*
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    saveFile(fileFormat);
    */

    QString initialPath = QDir::currentPath() + "/untitled.bin";
    if (!mEd.mapFilename.empty()) {
        initialPath = QString::fromStdString(mEd.mapFilename);
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save map"),
                               initialPath,
                               tr("All Files (*)"));
    if (fileName.isEmpty())
        return false;

    bool ok = SaveProject(mEd.proj, fileName);
    if (!ok) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Save map as failed"), tr("Poop. It's all gone pear-shaped."));
        return false;
    }
    mEd.mapFilename = fileName.toStdString();
    mEd.modified = false;
    return true;
}

void MainWindow::open()
{
#if 0
    QString initialPath = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open map"),
                               initialPath,
                               tr("All Files (*)"));
    if (fileName.isEmpty())
        return;

    Tilemap newMap;
    if (!ReadMap(newMap, fileName, 40, 25)) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Open map failed"), tr("Poop. It's all gone pear-shaped."));
        return;
    }

    mEd.map = newMap;
    // TODO: doesn't handle shrinking!
    MapRect all{TilePoint{0,0}, newMap.w, newMap.h};
    mEd.modified = false;
    mEd.mapFilename = fileName.toStdString();
    for (auto l : mEd->listeners) {
        l->ProjMapChanged(all);
    }
#endif
}

void MainWindow::importCharset()
{
    QString initialPath = QDir::currentPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Charset"),
                               initialPath,
                               tr("PNG Files (*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    Charset newTiles;
    if (!ImportCharset(fileName, newTiles, 8, 8)) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Import Charset failed"), tr("Poop. It's all gone pear-shaped."));
        return;
    }

    ReplaceCharsetCmd *cmd = new ReplaceCharsetCmd(mEd, newTiles);
    mEd.AddCmd(cmd);
}

void MainWindow::help()
{
    QMessageBox::about(this, tr("RetroMap Help"),
            tr("<p>blah blah blah</p>"));
}

void MainWindow::addMap()
{
    Tilemap& cur = mEd.proj.maps[mPresenter.CurrentMap()];
    MapSizeDialog dlg(this, cur.w, cur.h);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    Tilemap map;
    map.w = dlg.ResultW();
    map.h = dlg.ResultH();
    map.cells.resize(map.w * map.h);

    // insert it after current one.
    int n = mPresenter.CurrentMap() + 1;
    InsertMapsCmd* cmd = new InsertMapsCmd(mEd, {map}, n);
    mEd.AddCmd(cmd);
}

void MainWindow::resizeMap()
{
    int mapNum = mPresenter.CurrentMap();
    Tilemap& cur = mEd.proj.maps[mapNum];
    MapSizeDialog dlg(this, cur.w, cur.h);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    MapRect newSize(TilePoint(0, 0), dlg.ResultW(), dlg.ResultH());
    auto* cmd = new ResizeMapCmd(mEd, mapNum, newSize);
    mEd.AddCmd(cmd);
}


void MainWindow::exchangeMap()
{
    int mapNum = mPresenter.CurrentMap();
    MapExchangeDialog dlg(this, mapNum, mapNum);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
   auto* cmd = new ExchangeMapsCmd(mEd, dlg.ResultMap1(), dlg.ResultMap2());
   mEd.AddCmd(cmd);
}


void MainWindow::importMaps()
{
    QString initialPath = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import maps"),
                               initialPath,
                               tr("All Files (*)"));
    if (fileName.isEmpty())
        return;

    Proj donor;
    if (!LoadProject(donor, fileName)) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Import failed"), tr("Poop. It's all gone pear-shaped."));
        return;
    }

    // Insert them after current one (disregard palette, charset etc...)
    int n = mPresenter.CurrentMap() + 1;
    InsertMapsCmd* cmd = new InsertMapsCmd(mEd, donor.maps, n);
    mEd.AddCmd(cmd);
}


// ModelListener
void MainWindow::EditorPenChanged()
{
}

void MainWindow::EditorBrushChanged()
{
    mActions.useCustomBrush->setChecked(mEd.useBrush);
}


void MainWindow::EditorToolChanged()
{
    int kind = mEd.tool->Kind();
    switch (kind) {
    case TOOL_DRAW:
        if (!mActions.drawTool->isChecked()) {
            mActions.drawTool->setChecked(true);
        }
        break;
    case TOOL_PICKUP:
        if (!mActions.pickupTool->isChecked()) {
            mActions.pickupTool->setChecked(true);
        }
        break;
    case TOOL_RECT:
        if (!mActions.rectTool->isChecked()) {
            mActions.rectTool->setChecked(true);
        }
        break;
    case TOOL_FLOODFILL:
        if (!mActions.floodFillTool->isChecked()) {
            mActions.floodFillTool->setChecked(true);
        }
        break;
    case TOOL_ENT:
        if (!mActions.entTool->isChecked()) {
            mActions.entTool->setChecked(true);
        }
        break;
    }
}

// ModelListener
void MainWindow::ProjMapModified(int mapNum, MapRect const& dirty)
{
}

// ModelListener
void MainWindow::ProjCharsetModified()
{
    mCharsetWidget->SetTiles(&mEd.proj.charset, &mEd.proj.palette);
    RethinkTitle();
}

// ModelListener
void MainWindow::ProjMapsInserted(int first, int count)
{
    RethinkTitle();
}

// ModelListener
void MainWindow::ProjMapsRemoved(int first, int count)
{
    RethinkTitle();
}

