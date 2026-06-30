#include <QApplication>
#include <QPushButton>
#include "MainWindow.h"
#include "helpers.h"

#include "model.h"
#include "scripting.h"

int main(int argc, char **argv)
{
    // If -s or --script, run upon input files then exit. No GUI.
    {
        std::string script;
        std::vector<std::string> infiles;
        int i = 1;
        while(i < argc) {
            std::string arg(argv[i]);
            if (arg == "--script" || arg == "-s") {
                ++i;
                if (i >= argc) {
                    fprintf(stderr, "Missing param for --script/-s\n");
                    return 1;
                }
                script = argv[i];
            } else {
                infiles.push_back(arg);
            }
            ++i;
        }

        if (!script.empty()) {
            // Script file was specified. Run in CLI-only mode. No QT GUI stuff!
            for(auto infile : infiles) {
                Model model;
                if (!LoadProject(model.proj, infile.c_str())) {
                    fprintf(stderr, "Error loading %s\n", infile.c_str());
                    return 1;
                }
                model.mapFilename = infile;
                int result = RunScript(script.c_str(), model);
                if (result != 0) {
                    return result;
                }
            }
            return 0;  // Success!
        }
    }

    // If we get this far we're going full GUI.
    // Ignore the previous commandline parsing and let QT deal with it.

    QApplication app (argc, argv);

    std::vector<Model*> editors;

    auto args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        Proj proj;
        if (!LoadProject(proj, args.at(i))) {
            printf("ERROR: failed to load %s\n", args.at(i).toStdString().c_str());
            continue;
        }
        Model* ed = new Model();
        ed->proj = proj;
        ed->mapFilename = args.at(i).toStdString();
        editors.push_back(ed);
    }
    if(editors.empty()) {
        // blank
        Model* ed = new Model();
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
