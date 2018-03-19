#pragma once

#include "common.h"

#define INTERVAL_SCROLL 170 // msec

#define OPERATION_COPY    0
#define OPERATION_INJECT  2
#define OPERATION_FILL    3
#define OPERATION_SHA     4
#define OPERATION_FIND    5
#define OPERATION_DECRYPT 6
#define OPERATION_ENCRYPT 7

bool isMTmodEnabled();
void setMTmodEnabled(bool enable);

bool isBGOperationRunning();
void setBGOperationRunning(bool running);

bool isScriptRunning();
void setScriptRunning(bool running);

void setCurrentOperationId(int id);
char* getCurrentOperationStr();
