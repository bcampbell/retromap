#include "cmd.h"
#include "model.h"
#include "proj.h"
//#include "helpers.h"
#include "tool.h"


Model::Model()
{
    leftPen = {1,1,0};
    rightPen = {32,0,0};
    DefaultProj(&proj);
    tool = new DrawTool(*this);
}


Model::~Model()
{
    delete tool;
    tool = nullptr;
    while(!undoStack.empty()) {
        delete undoStack.back();
        undoStack.pop_back();
    }
    while(!redoStack.empty()) {
        delete redoStack.back();
        redoStack.pop_back();
    }
}

void Model::SetTool(int toolKind)
{
    Tool* newTool = nullptr;
    switch(toolKind) {
        case TOOL_DRAW: newTool = new DrawTool(*this); break;
        case TOOL_RECT: newTool = new RectTool(*this); break;
        case TOOL_PICKUP: newTool = new PickupTool(*this); break;
        case TOOL_FLOODFILL: newTool = new FloodFillTool(*this); break;
        case TOOL_ENT: newTool = new EntTool(*this); break;
//        case TOOL_BRUSH: newTool = new BrushTool(*this); break;
        default:
            assert(false);  // bad tool.
            return;
    }

    tool->Reset();
    delete tool;
    tool = newTool;

    for (auto l : listeners) {
        l->EditorToolChanged();
    }
}



// Adds a command to the undo stack, and calls its Do() fn
void Model::AddCmd(Cmd* cmd)
{
    //const int maxundos = 128;

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


void Model::Undo()
{
    if(undoStack.empty()) {
        return;
    }
    Cmd* cmd = undoStack.back();
    undoStack.pop_back();
    cmd->Undo();
    redoStack.push_back(cmd);
}

void Model::Redo()
{
    if(redoStack.empty())
        return;
    Cmd* cmd = redoStack.back();
    redoStack.pop_back();
    cmd->Do();
    undoStack.push_back(cmd);
}

