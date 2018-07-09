#include "multithread.h"
#include "common.h"
#include "ui.h"
#include "scripting.h"
#include "hid.h"
#include "fsutil.h"

static bool enable_MTmod = true;

// file operation
static bool task_left = false;
static char cur_task_str[32];

// script queue
static char script_queue[MAX_SCRIPTS][256];
static char script_queue_all[MAX_SCRIPTS][256];
static int script_queue_num = 0;
static int script_all_num = 0;

// queue handle
void push(const char* new_str, char queue[MAX_SCRIPTS][256], int* n) {
    snprintf(queue[(*n)++], 256, "%s", new_str);
}
void push_front(const char* new_str, char queue[MAX_SCRIPTS][256], int* n) {
    for (int i = *n; i >= 1; i--) {
        snprintf(queue[i], 256, "%s", queue[i-1]);
    }
    snprintf(queue[0], 256, "%s", new_str);
    (*n)++;
}
bool pop(char queue[MAX_SCRIPTS][256], int* n) {
    if (!n) return false;
    for (int i = 1; i < *n; i++) {
        snprintf(queue[i-1], 256, "%s", queue[i]);
    }
    (*n)--;
    return true;
}
#define PUSH_ALL(s) push(s, script_queue_all, &script_all_num)
#define PUSH_FRONT(s) push_front(s, script_queue_all, &script_all_num)
#define POP_ALL()  pop(script_queue_all, &script_all_num)
#define PUSH_QUEUE(s) push(s, script_queue, &script_queue_num)
#define POP_QUEUE() pop(script_queue, &script_queue_num)

// MTmod enable
bool IsMTmodEnabled() {
    return enable_MTmod;
}
void SetMTmodEnabled(bool enable) {
    enable_MTmod = enable;
}

bool IsTaskLeft() {
    return enable_MTmod && task_left;
}
// used in ShowProgress_mt()(ui.c) and PathMoveCopy()(fsutil.c)
void StartTask() {
    if (!enable_MTmod) return;
    if (script_all_num) return;
    
    task_left = true;
}
void FinishTask() {
    if (!enable_MTmod && task_left) {
        ShowPrompt(false, "MTmod Fatal\n \nMTmod disabled with task left");
        bkpt;
    }
    
    task_left = false;
}
void SetCurrentTaskStr(const char* str) {
    if (!enable_MTmod) return;
    snprintf(cur_task_str, 32, "%s%s", script_all_num ? "[script] " : "", str);
}
const char* GetCurrentTaskStr() {
    if (!enable_MTmod) return NULL;
    return cur_task_str;
}

int GetScriptNum() {
    if (!enable_MTmod) return 0;
    
    return script_all_num;
}
void checkTask();
void finishScript() {
    if (!enable_MTmod) return;
    if (script_queue_num <= 0) {
        ShowPrompt(false, "MTmod Fatal : no script");
        bkpt;
    }
    POP_ALL();
    POP_QUEUE();
    checkTask();
}
void checkTask() {
    if (script_queue_num) {
        ExecuteGM9Script(script_queue[0]);
        finishScript();
    }
}
void StartScript(const char* path) {
    if (!enable_MTmod) {
        snprintf(script_queue_all[0], 256, "%s", path);
        script_all_num = 1;
        ExecuteGM9Script(path);
        script_all_num = 0;
        return;
    }
    if (script_all_num >= MAX_SCRIPTS) {
        ShowPrompt(false, "Too many script tasks.");
        return;
    }
    if (!script_all_num || ShowPrompt(true, "Queue it ?\n<B> to run it immediately.")) {
        // queue it
        PUSH_ALL(path);
        PUSH_QUEUE(path);
        if (script_all_num == 1) checkTask(); // first task
    } else {
        char path_new[256];
        snprintf(path_new, 256, "%s", path);
        
        PUSH_FRONT(path);
        // run it immediately
        ExecuteGM9Script(path_new);
        POP_ALL();
    }
}

bool _MTassert() {
    if (IsTaskLeft()) {
        ShowPrompt(false, "Another file operation is running.\nCan't continue");
        return false;
    }
    return true;
}

// true if confirmed cancel
bool MTmodCancelConfirm(const char* str) {
    char script_str[64];
    if (script_all_num) {
        char path_str[32+1];
        TruncateString(path_str, script_queue_all[0], 32, 12);
        
        snprintf(script_str, 64, "from script :\n%s\n \n", path_str);
    } else *script_str = '\0';
    
    char msg_str[64];
    if (str)
        snprintf(msg_str, 64, "%s\n", str);
    else *msg_str = '\0';
    
    bool res = ShowPrompt(true, "%s%s%s detected. Cancel?", script_str, msg_str, IsMTmodEnabled() ? "R+Select" : "B button");
    return res;
}

void MTmodScriptList() {
    if (!enable_MTmod) return; // to be safe
    ClearScreenF(true, true, COLOR_STD_BG);
    int line_height = min(10, FONT_HEIGHT_EXT+2);
    int vertical_char = min(64, SCREEN_WIDTH_ALT/FONT_WIDTH_EXT);
    
    char display_str[MAX_SCRIPTS][64+1];
    for (int i = 0; i < script_all_num; i++) {
        char path_str[64+1];
        TruncateString(path_str, script_queue_all[i], vertical_char-5, 20);
        snprintf(display_str[i], 256, "%-2d : %s", i+1, path_str);
    }
    if (!script_all_num) snprintf(display_str[0], 256, "No script running");
    
    DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, "MTmod menu");
    DrawStringF(MAIN_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, "Background tasks are currently stopped");
    DrawStringF(MAIN_SCREEN, 0, line_height, COLOR_BLUE, COLOR_STD_BG, "X+Y : ???");
    
    while(1) {
        for (int i = 0; i < script_all_num || !i; i++) {
            DrawStringF(ALT_SCREEN, 0, line_height*(i+1), COLOR_STD_FONT, COLOR_STD_BG, "%-*s", vertical_char, display_str[i]);
        }
        u32 pad_state = InputWait(0);
        if (pad_state & BUTTON_B) break; 
        
        // secret menu
        if (pad_state == (BUTTON_X | BUTTON_Y)) {
            ClearScreenF(true, true, COLOR_STD_BG);
            u32 flags = OVERWRITE_ALL;
            PathCopy("0:/gm9/scripts", "V:/Unlock.gm9", &flags);
            ShowPrompt(false, "Nice !!");
            ShowPrompt(false, "A special script to unlock\n"
                              "the write permissions\n"
                              "is generated in 0:/gm9/scripts\n");
            ShowPrompt(false, "It works only on versions\n"
                              "between 1.5.0 and 1.6.3 and\n"
                              "MTmod 1.1 or below.\n \n"
                              "So it won't work on the version\n"
                              "you are currently running.");
            break;
        }
    }
    InputCheck(true, false, false);
    ClearScreenF(true, true, COLOR_STD_BG);
}
