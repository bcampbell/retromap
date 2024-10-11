#include <QApplication>
#include <QPushButton>
#include "mainwindow.h"
#include "helpers.h"
#include "editor.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    Editor ed;
    MainWindow fenster(nullptr, ed);
    fenster.show();

    return app.exec();
}


