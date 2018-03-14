#include "multithread.h"

static bool EnableMTmod = true;

bool isMTmodEnabled() {
	return EnableMTmod;
}

void setMTmodEnabled(bool enable) {
	EnableMTmod = enable;
}
