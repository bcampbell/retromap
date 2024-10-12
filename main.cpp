#include <QApplication>
#include <QPushButton>
#include "mainwindow.h"
#include "helpers.h"
#include "editor.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    std::vector<Editor> editors;

    auto args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        Proj proj;
        if (!LoadProject(proj, args.at(i))) {
            printf("ERROR: failed to load %s\n", args.at(i).toStdString().c_str());
            continue;
        }
        editors.push_back(Editor());
        editors.back().proj = proj;
        editors.back().mapFilename = args.at(i).toStdString();
    }
    if(editors.empty()) {
        // blank
        editors.push_back(Editor());
    }

    for (auto& ed : editors) {
        MainWindow *fenster = new MainWindow(nullptr, ed);
        fenster->show();
        // cleanup or not?
    }

    return app.exec();
}


