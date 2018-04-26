#include "config.h"
#include "hid.h"
#include "ui.h"
#include "fsutil.h"
#include "vff.h"

int screen_brightness = -1;
bool show_space = false;
bool use_jpn_clock = false;
char font_path[256] = { 0 };

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

bool SaveConfig() {
	DirCreate("0:/gm9", "config");
	return FileSetData("0:/gm9/config/brightness", &screen_brightness, sizeof(int) , 0, true) &&
		   FileSetData("0:/gm9/config/show_space", &show_space       , sizeof(bool), 0, true) &&
		   FileSetData("0:/gm9/config/jpn_clock" , &use_jpn_clock    , sizeof(bool), 0, true) &&
		   FileSetData("0:/gm9/config/font_path" , font_path         , 256         , 0, true);  
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
	const char* mainstr = "GodMode9 Configuration Menu";
	const int line_height = min(10, GetFontHeight() + 2);
    u32 sel = 0;
	
	u8 n_opt = 0;
	u32 option_brightness = ++n_opt - 1;
	u32 option_show_free = ++n_opt - 1;
	u32 option_jpn_clock = ++n_opt - 1;
	
	char* options[n_opt];
	options[option_brightness] = "Screen brightness";
	options[option_show_free ] = "Show free space";
	options[option_jpn_clock ] = "Use Japanese style clock";
	
    
    ClearScreenF(true, true, COLOR_STD_BG);
    DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, mainstr);
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
        u32 pad_state = InputWait(0);
        if (pad_state & BUTTON_DOWN) sel = (sel+1) % n_opt;
        else if (pad_state & BUTTON_UP) sel = (sel+n_opt-1) % n_opt;
        else if (pad_state & BUTTON_B) break;
		else if (pad_state & BUTTON_X) {
			*font_path = '\0';
		}
		else if (pad_state & (BUTTON_LEFT | BUTTON_RIGHT)) {
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
	
    SaveConfig();
    ClearScreenF(true, true, COLOR_STD_BG);
}
