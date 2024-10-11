#include "editor.h"
#include "proj.h"
#include "helpers.h"


Editor::Editor() {
    leftPen = {1,1,0};
    rightPen = {32,0,0};
    InitProj(&proj);
}


