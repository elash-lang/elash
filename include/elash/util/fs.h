#pragma once

#include <elash/util/pathview.h>
#include <elash/util/pathbuf.h>
#include <elash/util/strbuf.h>

bool el_fs_path_abs(ElPathView path, ElPathBuf* out_abs);
bool el_fs_read_file(ElPathView path, ElStringBuf* out);
bool el_fs_write_file(ElPathView path, ElStringView content);

bool el_fs_mkdir(ElPathView path);
bool el_fs_mkdir_all(ElPathView path);
bool el_fs_mkfile(ElPathView path);

bool el_fs_mkdir_if_not_exists(ElPathView path);
bool el_fs_mkfile_if_not_exists(ElPathView path);

bool el_fs_touch(ElPathView path);
bool el_fs_rm(ElPathView path);
bool el_fs_rm_recursive(ElPathView path);

typedef enum ElFileType {
    EL_FILE_REGULAR,
    EL_FILE_DIR,
    EL_FILE_SYMLINK,
    EL_FILE_OTHER,
    EL_FILE_NOT_FOUND,
} ElFileType;

bool el_fs_file_exists(ElPathView path);
ElFileType el_fs_get_file_type(ElPathView path);

bool el_fs_set_executable(ElPathView path, bool enabled);
