#pragma once

#include "common.h"

#define GODMODE_EXIT_REBOOT     0
#define GODMODE_EXIT_POWEROFF   1
#define GODMODE_NO_EXIT         2

u32 GodMode(int entrypoint);
u8 GM9HandleUserInput();
u32 ScriptRunner(int entrypoint);
