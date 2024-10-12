#include "mainwindow.h"

#include "helpers.h"
#include "mapwidget.h"
#include "palettewidget.h"
#include "penwidget.h"
#include "tilesetwidget.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent, Editor& ed)
    : QMainWindow(parent), mEd(ed)
{
    // TODO: read QCoreApplication::arguments()

    mEd.modified = false;

    mEd.listeners.insert(this);

    createActions();
    createMenus();
    createWidgets();
    setWindowTitle(tr("Retromap"));
    resize(500, 500);
    show();
}

MainWindow::~MainWindow() {
    mEd.listeners.erase(this);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
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
    mActions.importTileset = new QAction(tr("&Import tileset..."), this);
    connect(mActions.importTileset, &QAction::triggered, this, &MainWindow::importTileset);

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

    mActions.addMap = new QAction(tr("Add map"), this);
    mActions.addMap->setShortcut(QKeySequence(Qt::Key_A));
    connect(mActions.addMap, &QAction::triggered, this, &MainWindow::addMap);

    mActions.nextMap = new QAction(tr("Next map"), this);
    mActions.nextMap->setShortcut(QKeySequence(Qt::Key_Right));
    connect(mActions.nextMap, &QAction::triggered, this, &MainWindow::nextMap);

    mActions.prevMap = new QAction(tr("Previous map"), this);
    mActions.prevMap->setShortcut(QKeySequence(Qt::Key_Left));
    connect(mActions.prevMap, &QAction::triggered, this, &MainWindow::prevMap);
}


void MainWindow::createMenus()
{
    {
        QMenu* m = new QMenu(tr("&File"), this);
        m->addAction(mActions.importTileset);
        m->addAction(mActions.open);
        m->addSeparator();
        m->addAction(mActions.save);
        m->addAction(mActions.saveAs);
        m->addSeparator();
        m->addAction(mActions.exit);
        menuBar()->addMenu(m);
    }
    {
        QMenu* m = new QMenu(tr("&Map"), this);
        m->addAction(mActions.nextMap);
        m->addAction(mActions.prevMap);
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

    mMapWidget = new MapWidget(nullptr, mEd);

    mTilesetWidget = new TilesetWidget(nullptr, mEd.proj.tileset, mEd.proj.palette);
    mTilesetWidget->setLeftTile(mEd.leftPen.tile);
    mTilesetWidget->setRightTile(mEd.rightPen.tile);

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
        v->addWidget(mPenWidget, Qt::AlignLeading);
        v->addWidget(mPaletteWidget, Qt::AlignLeading);
        h->addLayout(v);
    }
    {
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setWidget(mTilesetWidget);
        h->addWidget(scroll, Qt::AlignLeading);
    }
    top->addLayout(h);
    setCentralWidget(central);


    // Wire it all up!

    connect(mTilesetWidget, &TilesetWidget::leftChanged, this, [self=this](int tile) {
        self->mEd.leftPen.tile = tile;
        for (auto l : self->mEd.listeners) {
            l->ProjPenChanged();
        }
    });

    connect(mTilesetWidget, &TilesetWidget::rightChanged, this, [self=this](int tile) {
        self->mEd.rightPen.tile = tile;
        for (auto l : self->mEd.listeners) {
            l->ProjPenChanged();
        }
    });


    connect(mPaletteWidget, &PaletteWidget::leftChanged, this, [self=this](int ink){
        self->mEd.leftPen.ink = ink;
        for (auto l : self->mEd.listeners) {
            l->ProjPenChanged();
        }
    });
    connect(mPaletteWidget, &PaletteWidget::rightChanged, this, [self=this](int ink){
        self->mEd.rightPen.ink = ink;
        for (auto l : self->mEd.listeners) {
            l->ProjPenChanged();
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

void MainWindow::importTileset()
{
#if 0
    QString initialPath = QDir::currentPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Tileset"),
                               initialPath,
                               tr("PNG Files (*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    Tileset newTiles;
    if (!ImportTileset(fileName, newTiles, 8, 8)) {
        // TODO: proper error message
        QMessageBox::critical(this, tr("Import Tileset failed"), tr("Poop. It's all gone pear-shaped."));
        return;
    }

    mEd->tileset = newTiles;
    for (auto l : mEd->listeners) {
        l->ProjTilesetChanged();
    }
#endif
}



#if 0
bool MainWindow::saveFile(const QByteArray &fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                               initialPath,
                               tr("%1 Files (*.%2);;All Files (*)")
                               .arg(QString::fromLatin1(fileFormat.toUpper()))
                               .arg(QString::fromLatin1(fileFormat)));
    if (fileName.isEmpty())
        return false;
    return scribbleArea->saveImage(fileName, fileFormat.constData());
}
#endif

void MainWindow::help()
{
    QMessageBox::about(this, tr("RetroMap Help"),
            tr("<p>blah blah blah</p>"));
}

void MainWindow::addMap()
{
    Tilemap map;
    map.w = 40;
    map.h = 25;
    map.cells.resize(map.w * map.h);

    // insert it after current one.
    int n = mMapWidget->CurrentMap() + 1;
    auto it = mEd.proj.maps.begin() + n;
    mEd.proj.maps.insert(it, map);
    for (auto l : mEd.listeners) {
        l->ProjMapsInserted(n, 1);
    }
    mMapWidget->SetCurrentMap(n);
}

void MainWindow::nextMap()
{
    int n = mMapWidget->CurrentMap() + 1;
    if (n < (int)mEd.proj.maps.size()) {
        mMapWidget->SetCurrentMap(n);
    }
}

void MainWindow::prevMap()
{
    int n = mMapWidget->CurrentMap() - 1;
    if (n >= 0) {
        mMapWidget->SetCurrentMap(n);
    }
}

// EditListener
void MainWindow::ProjPenChanged()
{
}

// EditListener
void MainWindow::ProjMapModified(int mapNum, MapRect const& dirty)
{
}

// EditListener
void MainWindow::ProjTilesetModified()
{
}

// EditListener
void MainWindow::ProjMapsInserted(int first, int count)
{
    // maybe should just be handled by mapwidget
    int cur = mMapWidget->CurrentMap();
    if (cur >= first) {
        mMapWidget->SetCurrentMap(cur + count);
    }
}

void MainWindow::ProjMapsRemoved(int first, int count)
{
    assert(false);  // TODO!
}

