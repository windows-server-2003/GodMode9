#pragma once

#include "common.h"

#define MAX_SCRIPTS 10

#define INTERVAL_SCROLL 170 // msec

bool IsMTmodEnabled();
void SetMTmodEnabled(bool enable);

bool IsTaskLeft();
void StartTask();
void FinishTask();
void SetCurrentTaskStr(const char* str);
const char* GetCurrentTaskStr();

int GetScriptNum();
void StartScript(const char* path);

#define MTassert() if (!_MTassert()) return 0
bool _MTassert();

bool MTmodCancelConfirm(const char* str);
void MTmodScriptList();
