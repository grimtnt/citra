// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

// Directory separators
#define DIR_SEP "/"
#define DIR_SEP_CHR '/'

// The user data dir
#define ROOT_DIR "."
#define USERDATA_DIR "user"
#ifdef USER_DIR
#define EMU_DATA_DIR USER_DIR
#else
#ifdef _WIN32
#define EMU_DATA_DIR "Citra"
#else
#define EMU_DATA_DIR "citra-emu"
#endif
#endif

// Subdirs in the User dir returned by GetUserPath(D_USER_IDX)
#define CONFIG_DIR "config"
#define CACHE_DIR "cache"
#define SDMC_DIR "sdmc"
#define NAND_DIR "nand"
#define SYSDATA_DIR "sysdata"
#define LOG_DIR "log"

// Filenames
// Files in the directory returned by GetUserPath(D_LOGS_IDX)
#define LOG_FILE "citra_log.txt"

// Sys files
#define SHARED_FONT "shared_font.bin"
#define AES_KEYS "aes_keys.txt"
