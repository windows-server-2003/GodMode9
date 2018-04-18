#include "config.h"
#include "hid.h"
#include "ui.h"
#include "fsutil.h"

int screen_brightness = -1;
bool show_space = false;
bool use_jpn_clock = false;

typedef struct {
	char signature[8]; // "GM9CONF"
	u64 reserved;
	int screen_brightness;
	bool show_space;
	bool use_jpn_clock;
} __attribute__ ((aligned)) Config;

int GetScreenBrightnessConfig() {
	return screen_brightness;
}

bool isSpaceShown() {
	return show_space;
}

bool isJapaneseClockUsed() {
	return use_jpn_clock;
}

bool validateConfig(Config* save) {
	bool ret = true;
	if (strncmp(save->signature, "GM9CONF", 7) != 0) ret = false;
	if (save->reserved != 0) ret = false;
	if (save->screen_brightness < -1 || save->screen_brightness > 15) ret = false;
	if (save->show_space != true && save->show_space != false) ret = false; 
	if (save->use_jpn_clock != true && save->use_jpn_clock != false) ret = false; 
	return ret;
}

bool SaveConfig() {
	Config* save = malloc(sizeof(Config));
	memcpy(save->signature, "GM9CONF", 8);
	save->reserved = 0;
	save->screen_brightness = screen_brightness;
	save->show_space = show_space;
	save->use_jpn_clock = use_jpn_clock;
	
	bool ret;
	ret = FileSetData("0:/gm9/config.bin", save, sizeof(Config), 0, true);
	
	free(save);
	return ret;
}

bool LoadConfig() {
	Config* save = malloc(sizeof(Config));
	
	bool ret;
	ret = (FileGetData("0:/gm9/config.bin", save, sizeof(Config), 0) == sizeof(Config));
	
	if (ret && validateConfig(save)) {
		screen_brightness = save->screen_brightness;
		show_space = save->show_space;
		use_jpn_clock = save->use_jpn_clock;
	}
	free(save);
	return ret;
}

void ConfigMenu() {
	const char* mainstr = "GodMode9 Configuration Menu";
	const int line_height = min(10, GetFontHeight() + 2);
    u32 sel = 0;
	
	u8 n_opt = 0;
	u32 option_brightness = ++n_opt;
	u32 option_show_free = ++n_opt;
	u32 option_jpn_clock = ++n_opt;
	
	char* options[n_opt];
	options[option_brightness - 1] = "Screen brightness";
	options[option_show_free  - 1] = "Show free space";
	options[option_jpn_clock  - 1] = "Use Japanese style clock";
	
    
    ClearScreenF(true, true, COLOR_STD_BG);
    DrawStringF(ALT_SCREEN, 0, 0, COLOR_GREEN, COLOR_STD_BG, mainstr);
    while (true) {
        for (u32 i = 0; i < n_opt; i++) {
			// option item
			DrawStringF(ALT_SCREEN, 0, ((line_height+2)*(i+1)), (sel == i) ? COLOR_STD_FONT : COLOR_LIGHTGREY, COLOR_STD_BG, options[i]);
			
			// value
			char* valstr_tmp = "";
			if (i == option_brightness - 1) {
				if (screen_brightness == -1) valstr_tmp = "Auto";
				else {
					char val[3 + 1];
					snprintf(val, 3, "%d", screen_brightness);
					valstr_tmp = val;
				}
			} else if (i == option_show_free - 1) {
				valstr_tmp = show_space ? "Enabled" : "Disabled";
			} else if (i == option_jpn_clock - 1) {
				valstr_tmp = use_jpn_clock ? "Enabled" : "Disabled";
			}
			
			// align to right
			char valstr[10 + 1];
			snprintf(valstr, 10, "%9.9s", valstr_tmp);
			DrawStringF(ALT_SCREEN, SCREEN_WIDTH(ALT_SCREEN) - GetFontWidth() * 9, ((line_height+2)*(i+1)),
				(sel == i) ? COLOR_STD_FONT : COLOR_LIGHTGREY, COLOR_STD_BG, valstr);
		}
        u32 pad_state = InputWait(0);
        if (pad_state & BUTTON_DOWN) sel = (sel+1) % n_opt;
        else if (pad_state & BUTTON_UP) sel = (sel+n_opt-1) % n_opt;
        else if (pad_state & BUTTON_B) break;
		else if (pad_state & (BUTTON_LEFT | BUTTON_RIGHT)) {
			if (sel == option_brightness - 1) {
				screen_brightness += ((pad_state & BUTTON_RIGHT) ? 1 : -1);
				if (screen_brightness < -1) screen_brightness = 15;
				else if (screen_brightness > 15) screen_brightness = -1;
			} else if (sel == option_show_free - 1) {
				show_space = !show_space;
			} else if (sel == option_jpn_clock - 1) {
				use_jpn_clock = !use_jpn_clock;
			}
		}
    }
	
    SaveConfig();
    ClearScreenF(true, true, COLOR_STD_BG);
}
