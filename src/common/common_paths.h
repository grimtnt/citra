// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

// Directory separators
#define DIR_SEP "/"
#define DIR_SEP_CHR '/'

// The user directory
#ifdef _WIN32
#define DATA_DIR "Citra"
#else
#define DATA_DIR "citra-emu"
#endif

// Subdirs in the user directory returned by GetUserPath(D_USER_IDX)
#define CONFIG_DIR "config"
#define SDMC_DIR "sdmc"
#define NAND_DIR "nand"
#define SYSDATA_DIR "sysdata"
#define LOG_DIR "log"

// Filenames
// Files in the directory returned by GetUserPath(D_LOGS_IDX)
#define LOG_FILE "citra_log.txt"

// System files
#define AES_KEYS "aes_keys.txt"
