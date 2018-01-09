#pragma once

// header signatures
#define _ZIP_LFH_SIG 0x04034B50
#define _ZIP_DD_SIG  0x4B500003

// errors
#define	ZIP_NO_ERRORS            0
#define	ZIP_USER_ABORT           1
#define	ZIP_ERROR_FAILED_READ    2
#define ZIP_ERROR_FAILED_CREATE  3
#define	ZIP_ERROR_FAILED_INJECT  4
#define	ZIP_ERROR_ZIP_TOO_SMALL  5
#define	ZIP_ERROR_NAME_TOO_LONG  6
#define	ZIP_ERROR_C              7 // compressed
#define	ZIP_ERROR_CE             8 // compressed or encrypted
#define	ZIP_ERROR_CE6            9 // compressed or encrypted or used ZIP64
#define	ZIP_ERROR_UNKNOWN       10

#define _ZIP_LFH_SIZE sizeof(ZipLocalFileHeader)

typedef struct {
	u32 signature;       // reserved (0x504B0304)
	u8  version;         // 0x0a:file 0x14:dir or deflated file
	u8  os;              // 0x00:DOS/OS2(FAT) 0x03:Unix/Linux 0x07:Mac 0x0a:Windows(NTFS) 0x13:OSX(Darwin)
	u16 options_flags;   // bit0 : password, bit3 : Data descriptor, bit11 : UTF-8 as file names and comments
	u16 comp_method;     // 0x0000:non-compressed 0x0008:Deflated
	u16 last_mod_time;   // (bit0-4)*2:seconds bit5-10:minutes bit11-15:hours
	u16 last_mod_date;   // bit0-4:days bit5-8:months (bit9-15)+1980:years
	u32 crc32;           // file's CRC32
	u32 size_compressed; // file's size after compressing
	u32 size_original;   // file's original size
	u16 name_len;        // the length of file(dir)'s name
	u16 extra_field_len; // 0 if no extra fields
} __attribute__((packed)) ZipLocalFileHeader;

bool ZipExtract(const char* path, const char* extrpath, u32* flags);
