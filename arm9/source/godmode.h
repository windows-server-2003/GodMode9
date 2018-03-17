#pragma once

#include "common.h"

#define GODMODE_EXIT_REBOOT     0
#define GODMODE_EXIT_POWEROFF   1
#define GODMODE_NO_EXIT         2

#define GODMODE_MODE_NORMAL 0 // foreground
#define GODMODE_MODE_BG     1 // background(don't check Home/Power button)
#define GODMODE_MODE_BG_MCU 2 // background(check Home/Power button)

u32 GodMode(int entrypoint);
u8 GM9HandleUserInput(u8 mode);
u32 ScriptRunner(int entrypoint);
