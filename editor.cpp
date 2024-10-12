#include "cmd.h"
#include "editor.h"
#include "proj.h"
#include "helpers.h"


Editor::Editor()
{
    leftPen = {1,1,0};
    rightPen = {32,0,0};
    InitProj(&proj);
}


Editor::~Editor()
{
    while(!undoStack.empty()) {
        delete undoStack.back();
        undoStack.pop_back();
    }
    while(!redoStack.empty()) {
        delete redoStack.back();
        redoStack.pop_back();
    }
}


// TODO: move undo stack onto Project!
// Adds a command to the undo stack, and calls its Do() fn
void Editor::AddCmd(Cmd* cmd)
{
    const int maxundos = 128;

    undoStack.push_back(cmd);
    if(cmd->State() == Cmd::NOT_DONE) {
        cmd->Do();
    }

    // adding a new command renders the redo stack obsolete.
    while(!redoStack.empty()) {
        delete redoStack.back();
        redoStack.pop_back();
    }
/*
    // limit amount of undos to something reasonable.
    int trimcount = undoStack.size() - maxundos;
    while(trimcount > 0) {
        delete undoStack.front();
        undoStack.pop_front();
        --trimcount;
    }
*/
}


void Editor::Undo()
{
    if(undoStack.empty()) {
        return;
    }
    Cmd* cmd = undoStack.back();
    undoStack.pop_back();
    cmd->Undo();
    redoStack.push_back(cmd);
}

void Editor::Redo()
{
    if(redoStack.empty())
        return;
    Cmd* cmd = redoStack.back();
    redoStack.pop_back();
    cmd->Do();
    undoStack.push_back(cmd);
}

