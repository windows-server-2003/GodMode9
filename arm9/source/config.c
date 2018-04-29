#include "config.h"
#include "hid.h"
#include "ui.h"
#include "fsutil.h"
#include "vff.h"
#include "support.h"

static int screen_brightness = -1;
static bool show_space = false;
static bool use_jpn_clock = false;
static char font_path[256] = { 0 };


// getters
int GetScreenBrightnessConfig() { return screen_brightness; }
bool isSpaceShown()             { return show_space; }
bool isJapaneseClockUsed()      { return use_jpn_clock; }
char* GetFontPathConfig()       { return font_path; }

// setters
void SetFontPathConfig(const char* path) {
    if (path)
        snprintf(font_path, 255, path);
    else *font_path = '\0';
    SaveConfig();
}

void fixConfig() {
    if (screen_brightness < -1 || screen_brightness > 15) screen_brightness = -1;
    if (strnlen(font_path, 256) == 256) *font_path = '\0';
}

void initConfig() {
    screen_brightness = -1;
    show_space = false;
    use_jpn_clock = false;
    *font_path = '\0';
}

bool SaveConfig() {
    DirCreate("0:/gm9", "config");
    bool ret = FileSetData("0:/gm9/config/brightness", &screen_brightness, sizeof(int) , 0, true) &&
			   FileSetData("0:/gm9/config/show_space", &show_space       , sizeof(bool), 0, true) &&
		   	   FileSetData("0:/gm9/config/jpn_clock" , &use_jpn_clock    , sizeof(bool), 0, true) &&
               FileSetData("0:/gm9/config/font_path" , font_path         , 256         , 0, true);
			   
	if (!ret) ShowPrompt(false, "Failed to save settings");
	return ret;
}

bool LoadConfig() {
    bool ret = FileGetData("0:/gm9/config/brightness", &screen_brightness, sizeof(int) , 0) == sizeof(int)  &&
               FileGetData("0:/gm9/config/show_space", &show_space       , sizeof(bool), 0) == sizeof(bool) &&
               FileGetData("0:/gm9/config/jpn_clock" , &use_jpn_clock    , sizeof(bool), 0) == sizeof(bool) &&
               FileGetData("0:/gm9/config/font_path" , font_path         , 256         , 0) == 256;
    
    if (!ret) { // create new files
        fixConfig();
        return SaveConfig();
    }
    
    return ret;
}

void ConfigMenu() {
    const int line_height = min(10, GetFontHeight() + 2);
    u32 sel = 0;
	bool edited = false;
    
    u8 n_opt = 0;
    u32 option_brightness = ++n_opt - 1;
    u32 option_show_free = ++n_opt - 1;
    u32 option_jpn_clock = ++n_opt - 1;
    
    char* options[n_opt];
    options[option_brightness] = "Screen brightness";
    options[option_show_free ] = "Show free space";
    options[option_jpn_clock ] = "Use Japanese style clock";
	
    ClearScreenF(true, true, COLOR_STD_BG);
	char instr[256];
	snprintf(instr, 255, "GodMode9 Configuration Menu\n \nSTART - Save settings\nY - Reset font\nX - Reset selected setting to default\nR+X - Reset all settings to default");

    ShowString(instr);
	DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, "GodMode9 Configuration Menu");
    while (true) {
        for (u32 i = 0; i < n_opt; i++) {
            // option item
            DrawStringF(ALT_SCREEN, 0, (line_height+2)*(i+1), (sel == i) ? COLOR_STD_FONT : COLOR_LIGHTGREY, COLOR_STD_BG, options[i]);
            
            // value
            char valstr_tmp [16];
            if (i == option_brightness) {
                if (screen_brightness == -1) strncpy(valstr_tmp, "Auto", 15);
                else snprintf(valstr_tmp, 15, "%d", screen_brightness);
            } else if (i == option_show_free) {
                strncpy(valstr_tmp, show_space ? "Enabled" : "Disabled", 15);
            } else if (i == option_jpn_clock) {
                strncpy(valstr_tmp, use_jpn_clock ? "Enabled" : "Disabled", 15);
            }
            
            // align to right
            char valstr[16];
            snprintf(valstr, 15, "%9.9s", valstr_tmp);
            DrawStringF(ALT_SCREEN, SCREEN_WIDTH(ALT_SCREEN) - GetFontWidth() * 9, (line_height+2)*(i+1),
                (sel == i) ? COLOR_STD_FONT : COLOR_LIGHTGREY, COLOR_STD_BG, valstr);
        }
		
        u32 pad_state = InputWait(3);
		
        if (pad_state & BUTTON_DOWN) sel = (sel+1) % n_opt;
        else if (pad_state & BUTTON_UP) sel = (sel+n_opt-1) % n_opt;
        else if (pad_state & BUTTON_B) { // exit
			if (edited) {
				if (ShowPrompt(true, "You have edited some settings.\nDo you want to save them ?")) {
					if (!SaveConfig()) ShowPrompt(false, "Failed to save settings");
				} else LoadConfig();
			}
			break;
		}
		else if (pad_state & BUTTON_START) { // save setttings
			if (SaveConfig()) {
				ShowPrompt(false, "All settings saved");
				edited = false;
			} else ShowPrompt(false, "Failed to save settings");
		}
        else if (pad_state & BUTTON_Y) { // reset font
            *font_path = '\0';
            if (CheckSupportFile("font.pbm")) {
                u8* pbm = (u8*) malloc(0x10000); // arbitrary, should be enough by far
                if (pbm) {
                    u32 pbm_size = LoadSupportFile("font.pbm", pbm, 0x10000);
                    if (pbm_size) SetFontFromPbm(pbm, pbm_size);
                    free(pbm);
                }
            } else SetFontFromPbm(NULL, 0); // from VRAM
			
			// redraw because font might be changed
			ShowString(instr);
			DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, "GodMode9 Configuration Menu");
			
			SaveConfig();
        }
		else if (pad_state & BUTTON_X && pad_state & BUTTON_R1) { // reset all settings
			if (ShowPrompt(true, "Do you really want to\nreset all setttings to default ?")) {
				initConfig();
				
				// redraw because font might be changed
				ShowString(instr);
				DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, "GodMode9 Configuration Menu");
				
				SaveConfig();
			}
		}
		else if (pad_state & BUTTON_X) { // reset current selected item to the default
			if (sel == option_brightness) screen_brightness = -1;
			else if (sel == option_show_free) show_space = false;
			else if (sel == option_jpn_clock) use_jpn_clock = false;
			edited = true;
		}
        else if (pad_state & (BUTTON_LEFT | BUTTON_RIGHT)) { // change value
			edited = true;
            if (sel == option_brightness) {
                screen_brightness += ((pad_state & BUTTON_RIGHT) ? 1 : -1);
                if (screen_brightness < -1) screen_brightness = 15;
                else if (screen_brightness > 15) screen_brightness = -1;
            } else if (sel == option_show_free) {
                show_space = !show_space;
            } else if (sel == option_jpn_clock) {
                use_jpn_clock = !use_jpn_clock;
            }
        }
    }
	
    ClearScreenF(true, true, COLOR_STD_BG);
}
