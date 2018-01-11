#include "fsutil.h"
#include "ui.h"
#include "vff.h"
#include "fsperm.h"
#include "zip.h"

u8 zipExtractContent(const char* path, const char* extrpath, ZipLocalFileHeader hdr, u32 lfh_ptr, u32* flags) {
    u32 data_start_off;     // start offset of adtually data
    bool isdir;
    // these variables here are some path
    char name [256] = { 0 }; // content name (e.g. File : "testdir/testfile.txt" Dir : "testdir/testsubdir") (last slash will be removed in the process)
    char* real_name = NULL;         // the real name of the content (e.g. File : "testfile.txt" Dir : "testsubdir")
    char extr_path_full[256] = { 0 }; // the full path of the file (e.g. File : "0:/extract/testdir/testfile.txt" Dir : "0:/extract/testdir/testsubdir" (but not used))
    char extr_dir_full [256] = { 0 }; // the full path of dir of content or full path of the folder(e.g. File : "0:/extract/testdir" Dir : "0:/extract/testdir/testsubdir")
    
    if (hdr.size_compressed != hdr.size_original) return ZIP_ERROR_C; // if not compressed, they are the same
   
    // get file name
    if (hdr.name_len > 255) return ZIP_ERROR_NAME_TOO_LONG; // check file name length
    if (FileGetData(path, name, hdr.name_len, lfh_ptr + _ZIP_LFH_SIZE) != hdr.name_len) return ZIP_ERROR_FAILED_READ;
    
    // get if it is a file (by checking content name end char)
    char name_last = name[strlen(name)-1]; // last char : '/' means it must be a dir
    if (name_last == '/') isdir = true;
    else isdir = false;
    
    // check zip version
    if (hdr.version == 0x0a) { // a non-compressed file
        if (isdir) { // mismatch (the name ends with '/', but the version to extract says it is a file)
            if (hdr.size_compressed != 0) { // This means the content maybe a file, so handle it as so
                isdir = false;
                name[strlen(name)-1] = '\0'; // remove '/' at the end of the file name
            } // else : must be just a directory and the version is wrong
        }
    } else if (hdr.version == 0x14) { // a dir or a compressed/deflated file
        if (!isdir) return ZIP_ERROR_CE; // the file is compressed or encrypted, can't extract.
    } else return ZIP_ERROR_CE6; // the content is encrypted, compressed or used ZIP64, can't extract.
    
    // get the start offset of actually data
    data_start_off = lfh_ptr + _ZIP_LFH_SIZE + hdr.name_len + hdr.extra_field_len;
    
    // search for last '/' in content full name
    snprintf(extr_path_full, 255, "%s/%s", extrpath, name);
    char* slash = strrchr(extr_path_full, '/');
    if (slash) {
        real_name = slash+1;
        strncpy(extr_dir_full, extr_path_full, slash-extr_path_full);
    } else return ZIP_ERROR_UNKNOWN; // no slash in the full path(impossible)
    
    fvx_rmkdir(extr_dir_full); // create dir
    if (!isdir) { // if file, create file and inject the data
        if (PathExist(extr_path_full)) { // path already exists
            if (*flags & SKIP_ALL) return ZIP_NO_ERRORS;
            if (flags && !(*flags & (OVERWRITE_CUR|OVERWRITE_ALL))) { // not overwrite
                const char* optionstr[5] =
                    {"Choose new name", "Overwrite it", "Skip it", "Overwrite all", "Skip all"};
                u32 user_select = ShowSelectPrompt(5, optionstr,
                    "Path already exists:\n%s", extr_path_full);
                if (user_select == 1) {
                    do {
                        if (ShowStringPrompt(real_name, 255 - (strlen(extr_dir_full)), "Choose new destination name")) {
                            snprintf(extr_path_full, 256, "%s/%s", extrpath, real_name); // update path (only used one below here)
                        }
                    } while (PathExist(extr_path_full));
                } else if (user_select == 2) {
                    *flags |= OVERWRITE_CUR;
                } else if (user_select == 3) {
                    return ZIP_NO_ERRORS;
                } else if (user_select == 4) {
                    *flags |= OVERWRITE_ALL;
                } else if (user_select == 5) {
                    *flags |= SKIP_ALL;
                    return ZIP_NO_ERRORS;
                } else {
                    return ZIP_USER_ABORT; // user abort
                }
            }else if (*flags & (OVERWRITE_CUR)) { // overwrite current but not all
                *flags &= ~OVERWRITE_CUR;
            }
            PathDelete(extr_path_full);
        }
        
        if (!FileCreateDummy(extr_dir_full, real_name, hdr.size_original)) return ZIP_ERROR_FAILED_CREATE; // since once deleted the dir, the file shouldn't be exist
        if (FileGetSize(path) < data_start_off + hdr.size_compressed) return ZIP_ERROR_ZIP_TOO_SMALL; // Check the archive size to prevent show error in FileInjectFile()
		if (hdr.size_compressed != 0) {
			if (!FileInjectFile(extr_path_full, path, 0, data_start_off, hdr.size_original, NULL)) return ZIP_ERROR_FAILED_INJECT;
		}
    }
    return ZIP_NO_ERRORS; // succeed
}

bool ZipExtract(const char* path, const char* extrpath, u32* flags) {
    bool silent = (flags && (*flags & SILENT));
    ZipLocalFileHeader hdr;
    u32 lfh_ptr = 0;
    u32 err_code = 0;
    
    // make sure there is a directory to extract(and also check write permissions)
    if (!CheckDirWritePermissions(extrpath)) return false; // once check here and never check below here
    fvx_rmkdir(extrpath);
    
    while (true) {
        // local File Header check
        if (FileGetData(path, &hdr, _ZIP_LFH_SIZE, lfh_ptr) != _ZIP_LFH_SIZE) return false; // reached the end of the file without End-of-central-dir-header
        if (hdr.signature != _ZIP_LFH_SIG) { // not a LFH
            if (hdr.signature == _ZIP_DD_SIG) return false; // data descriptor is not supported
            return true; // reached at the end of LFHs
        }
        
        err_code = zipExtractContent(path, extrpath, hdr, lfh_ptr, flags);
        if (err_code == ZIP_ERROR_FAILED_READ) { // unexpected file reading error
            if (!silent) ShowPrompt(false, "Error\noffset:%x\nFile i/o error", lfh_ptr);
            return false;
        }else if (err_code == ZIP_USER_ABORT) { // user abort
            return false;
        }else if (err_code != ZIP_NO_ERRORS) { // continuable error
			// debug
			if (err_code == ZIP_ERROR_FAILED_INJECT) {
				ShowPrompt(false, "Debug\n \noffset : %lx\nLFH:%lx\n", lfh_ptr, hdr.signature);
			}
            if (!silent) {
                if (!ShowPrompt(true, "Error occurred\n\nContinue to next content?\noffset:%lx", lfh_ptr)) return false;
            }
        }
        
        lfh_ptr += (_ZIP_LFH_SIZE + hdr.name_len + hdr.size_compressed + hdr.extra_field_len); // update file pointer
    }
}
