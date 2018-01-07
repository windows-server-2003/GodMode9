#pragma once

#include "fsutil.h"
#include "ui.h"
#include "vff.h"
#include "fsperm.h"

#define _ZIP_LFH_SIG 0x04034B50
#define _ZIP_DD_SIG  0x4B500003

#define _ZIP_LFH_SIZE 0x1E

typedef struct {
	u32 signature;       // reserved (0x504B0304)
	u8  version;         // 0x0a:file 0x14:dir or deflated file
	u8  os;              // 0x00:DOS/OS2(FAT) 0x03:Unix/Linux 0x07:Mac 0x0a:Windows(NTFS) 0x13:OSX(Darwin)
	u16 options_flags;   // bit0 : password, bit3 : Data descriptor, bit11 : UTF-8 as file names and comments
	u16 comp_method;     // 0x00:non-compressed 0x08:Deflated
	u16 last_mod_time;   // (bit0-4)*2:seconds bit5-10:minutes bit11-15:hours
	u16 last_mod_date;   // bit0-4:days bit5-8:months (bit9-15)+1980:years
	u32 crc32;           // file's CRC32
	u32 size_compressed; // file's size after compressing
	u32 size_original;   // file's original size
	u16 name_len;        // the length of file(dir)'s name
	u16 extra_field_len; // 0 if no extra fields
} ZipLocalFileHeader;

// bool ZipExtractLeg(const char* path, const char* extrpath, u32* flags);
bool ZipExtract(const char* path, const char* extrpath, u32* flags);
