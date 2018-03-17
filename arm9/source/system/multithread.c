#include "multithread.h"

static bool EnableMTmod = true;
static bool BGOperationRunning = false;
static bool ScriptRunning = false;

bool isMTmodEnabled() {
    if (ScriptRunning) return false; // disable MTmod while a script is running
    else return EnableMTmod;
}

void setMTmodEnabled(bool enable) {
    if (!ScriptRunning) EnableMTmod = enable;
}

bool isBGOperationRunning() {
    return BGOperationRunning;
}

void setBGOperationRunning(bool running) {
    BGOperationRunning = running;
}

void setScriptRunning(bool running) {
    ScriptRunning = running;
}
