#include "multithread.h"

static bool EnableMTmod = true;
static bool BGOperationRunning = false;

bool isMTmodEnabled() {
    return EnableMTmod;
}

void setMTmodEnabled(bool enable) {
    EnableMTmod = enable;
}

bool isBGOperationRunning() {
    return BGOperationRunning;
}

void setBGOperationRunning(bool running) {
    BGOperationRunning = running;
}
