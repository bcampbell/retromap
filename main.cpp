#include <QApplication>
#include <QPushButton>
#include "mainwindow.h"
#include "helpers.h"
#include "editor.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    std::vector<Editor*> editors;

    auto args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        Proj proj;
        if (!LoadProject(proj, args.at(i))) {
            printf("ERROR: failed to load %s\n", args.at(i).toStdString().c_str());
            continue;
        }
        Editor* ed = new Editor();
        ed->proj = proj;
        ed->mapFilename = args.at(i).toStdString();
        editors.push_back(ed);
    }
    if(editors.empty()) {
        // blank
        Editor* ed = new Editor();
        editors.push_back(ed);
    }

    for (auto ed : editors) {
        MainWindow *fenster = new MainWindow(nullptr, *ed);
        fenster->show();

        // TODO: need to clean up fenster?
    }

    auto status = app.exec();

    // cleanup.
    for (auto ed : editors) {
        delete ed;
    }

    return status;
}


