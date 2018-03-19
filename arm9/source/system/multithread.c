#include "multithread.h"

static bool EnableMTmod = true;
static bool BGOperationRunning = false;
static int curren_operation = -1;
static bool script_running = false;

bool isMTmodEnabled() {
    return EnableMTmod;
}
void setMTmodEnabled(bool enable) {
    EnableMTmod = enable;
}

bool isBGOperationRunning() {
    return BGOperationRunning;
}
// used in ShowProgress_mt()(ui.c) and PathMoveCopy()(fsutil.c)
void setBGOperationRunning(bool running) {
    BGOperationRunning = running;
}

bool isScriptRunning() {
    return script_running;
}
void setScriptRunning(bool running) {
    script_running = running;
}

void setCurrentOperationId(int id) {
    curren_operation = id;
}

char* getCurrentOperationStr() {
    if (curren_operation == OPERATION_COPY) return script_running ? "[script] Copying" : "Copying";
    else if (curren_operation == OPERATION_INJECT) return script_running ? "[script] Injecting" : "Injecting";
    else if (curren_operation == OPERATION_FILL) return script_running ? "[script] Filling" : "Filling";
    else if (curren_operation == OPERATION_SHA) return script_running ? "[script] Calculating sha" : "Calculating sha";
    else if (curren_operation == OPERATION_FIND) return script_running ? "[script] Searching data" : "Searching data";
    else if (curren_operation == OPERATION_DECRYPT) return script_running ? "[script] Decrypting" : "Decrypting";
    else if (curren_operation == OPERATION_ENCRYPT) return script_running ? "[script] Encrypting" : "Encrypting";
    else return NULL;
}
