#pragma once

#include "fsutil.h"
#include "ui.h"
#include "vff.h"
#include "fsperm.h"

u32 ZipExtractContent(const char* path, const char* extrpath, u32 CO, u32* newCO, char* ErrorDesc, u32* flags);
bool ZipExtract(const char* path, const char* extrpath, u32* flags);