#include "mainwindow.h"

#include "cmd.h"
#include "helpers.h"
#include "mapwidget.h"
#include "mapsizedialog.h"
#include "palettewidget.h"
#include "penwidget.h"
#include "charsetwidget.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QToolBar>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent, Editor& ed)
    : QMainWindow(parent), mEd(ed), mPresenter(ed)
{
    mEd.modified = false;

    mEd.listeners.insert(this);
    createActions();
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

    mActions.addMap = new QAction(tr("Add map..."), this);
    mActions.addMap->setShortcut(QKeySequence(Qt::Key_A));
    connect(mActions.addMap, &QAction::triggered, this, &MainWindow::addMap);

    // navigate around maps
    {
        QAction* a;
        a = new QAction(tr("North"), this);
        a->setShortcut(QKeySequence(Qt::Key_Up));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(0, -1);
            RethinkTitle();
        });
        mActions.mapNorth = a;
        
        a = new QAction(tr("South"), this);
        a->setShortcut(QKeySequence(Qt::Key_Down));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(0, 1);
            RethinkTitle();
        });
        mActions.mapSouth = a;

        a = new QAction(tr("West"), this);
        a->setShortcut(QKeySequence(Qt::Key_Left));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(-1, 0);
            RethinkTitle();
        });
        mActions.mapWest = a;

        a = new QAction(tr("East"), this);
        a->setShortcut(QKeySequence(Qt::Key_Right));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNav2D(1, 0); RethinkTitle();
        });
        mActions.mapEast = a;

        a = new QAction(tr("Next"), this);
        a->setShortcut(QKeySequence(Qt::Key_Plus));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNavLinear(1); RethinkTitle();
        });
        mActions.mapNext = a;

        a = new QAction(tr("Previous"), this);
        a->setShortcut(QKeySequence(Qt::Key_Minus));
        connect(a, &QAction::triggered, [&](){
            mPresenter.MapNavLinear(-1); RethinkTitle();
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

        mActions.pickupTool = a = new QAction(tr("Pick up brush"), toolGroup);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::Key_B));
        connect(a, &QAction::triggered, this, [self=this]() {
            self->mEd.SetTool(TOOL_PICKUP);
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

}


void MainWindow::createMenus()
{
    {
        QMenu* m = new QMenu(tr("&File"), this);
        m->addAction(mActions.importCharset);
        m->addAction(mActions.open);
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
        m->addAction(mActions.pickupTool);
        m->addSeparator();
        m->addAction(mActions.useCustomBrush);
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

    mMapWidget = new MapWidget(nullptr);

    mCharsetWidget = new CharsetWidget(nullptr);

    mCharsetWidget->SetTiles(&mEd.proj.charset, &mEd.proj.palette);

    mCharsetWidget->setLeftTile(mEd.leftPen.tile);
    mCharsetWidget->setRightTile(mEd.rightPen.tile);

    mPenWidget = new PenWidget(nullptr, mEd);

    mPaletteWidget = new PaletteWidget(nullptr, mEd.proj.palette);
    mPaletteWidget->setLeftColour(mEd.leftPen.ink);
    mPaletteWidget->setRightColour(mEd.rightPen.ink);

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
        toolbar->addAction(mActions.pickupTool);
        v->addWidget(toolbar, Qt::AlignLeading);
        v->addWidget(mPenWidget, Qt::AlignLeading);
        v->addWidget(mPaletteWidget, Qt::AlignLeading);
        h->addLayout(v);
    }
    {
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setWidget(mCharsetWidget);
        h->addWidget(scroll, Qt::AlignLeading);
    }
    top->addLayout(h);
    setCentralWidget(central);


    // Wire it all up!

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
    mPresenter.SetCurrentMap(n);
    RethinkTitle();
}

// EditListener
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
        if (mActions.pickupTool->isChecked()) {
            mActions.pickupTool->setChecked(true);
        }
        break;
    }
}

// EditListener
void MainWindow::ProjMapModified(int mapNum, MapRect const& dirty)
{
}

// EditListener
void MainWindow::ProjCharsetModified()
{
    mCharsetWidget->SetTiles(&mEd.proj.charset, &mEd.proj.palette);
}

// EditListener
void MainWindow::ProjMapsInserted(int first, int count)
{
}

// EditListener
void MainWindow::ProjMapsRemoved(int first, int count)
{
}

