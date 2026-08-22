#include <elash/defs/platform.h>

#if EL_PLATFORM_IS_WINDOWS

#include <elash/util/fs.h>
#include <elash/util/pathview.h>
#include <elash/util/pathbuf.h>

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool el_fs_path_abs(ElPathView path, ElPathBuf* out_abs) {
    if (path.len == 0) return false;

    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    char buffer[MAX_PATH];
    DWORD res = GetFullPathNameA(cpath, MAX_PATH, buffer, NULL);

    if (res == 0) {
        free(cpath);
        return false;
    }

    if (res >= MAX_PATH) {
        char* heap_buffer = malloc(res);
        if (!heap_buffer) {
            free(cpath);
            return false;
        }
        DWORD res2 = GetFullPathNameA(cpath, res, heap_buffer, NULL);
        free(cpath);
        if (res2 == 0 || res2 >= res) {
            free(heap_buffer);
            return false;
        }
        el_strbuf_clear(out_abs);
        bool success = el_strbuf_append(out_abs, el_sv_from_cstr(heap_buffer));
        free(heap_buffer);
        return success;
    }

    free(cpath);
    el_strbuf_clear(out_abs);
    return el_strbuf_append(out_abs, el_sv_from_cstr(buffer));
}

bool el_fs_read_file(ElPathView path, ElStringBuf* out) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    free(cpath);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return false;
    }

    if (!el_strbuf_resize(out, (usize)size.QuadPart)) {
        CloseHandle(hFile);
        return false;
    }

    DWORD nread;
    BOOL res = ReadFile(hFile, out->data, (DWORD)size.QuadPart, &nread, NULL);
    CloseHandle(hFile);

    if (!res || nread != (DWORD)size.QuadPart) {
        return false;
    }

    return true;
}

bool el_fs_write_file(ElPathView path, ElStringView content) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    free(cpath);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD nwritten;
    BOOL res = WriteFile(hFile, content.data, (DWORD)content.len, &nwritten, NULL);
    CloseHandle(hFile);

    return res && nwritten == (DWORD)content.len;
}

bool el_fs_mkdir(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    BOOL res = CreateDirectoryA(cpath, NULL);
    if (res) {
        free(cpath);
        return true;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        DWORD attrs = GetFileAttributesA(cpath);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            free(cpath);
            return true;
        }
    }

    free(cpath);
    return false;
}

bool el_fs_mkdir_all(ElPathView path) {
    if (path.len == 0) return true;
    if (el_fs_mkdir(path)) return true;

    ElPathView parent = el_pathview_dirname(path);
    if (parent.len == 0 || el_sv_eql(parent, path)) {
        return false;
    }

    if (!el_fs_mkdir_all(parent)) {
        return false;
    }

    return el_fs_mkdir(path);
}

bool el_fs_mkfile(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    CloseHandle(hFile);
    free(cpath);
    return true;
}

bool el_fs_mkdir_if_not_exists(ElPathView path) {
    return el_fs_mkdir(path);
}

bool el_fs_mkfile_if_not_exists(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_EXISTS || GetLastError() == ERROR_ALREADY_EXISTS) {
            free(cpath);
            return true;
        }
        free(cpath);
        return false;
    }

    CloseHandle(hFile);
    free(cpath);
    return true;
}

bool el_fs_touch(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    BOOL res = SetFileTime(hFile, NULL, &ft, &ft);

    CloseHandle(hFile);
    free(cpath);
    return res != 0;
}

bool el_fs_rm(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    DWORD attrs = GetFileAttributesA(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        free(cpath);
        return false;
    }

    BOOL res;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        res = RemoveDirectoryA(cpath);
    } else {
        res = DeleteFileA(cpath);
    }

    free(cpath);
    return res != 0;
}

static bool rm_recursive_internal(ElPathBuf* pb) {
    ElPathView current_pv = el_pathbuf_view(pb);
    char* cpath = el_sv_to_cstr_alloc(current_pv);
    if (cpath == NULL) return false;

    DWORD attrs = GetFileAttributesA(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        free(cpath);
        return false;
    }

    if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        BOOL res = DeleteFileA(cpath);
        free(cpath);
        return res != 0;
    }

    ElPathBuf search_pb;
    el_pathbuf_init_from(&search_pb, current_pv);
    el_pathbuf_join(&search_pb, EL_SV("*"));
    char* csearch = el_sv_to_cstr_alloc(el_pathbuf_view(&search_pb));
    el_pathbuf_destroy(&search_pb);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(csearch, &fd);
    free(csearch);

    if (hFind == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
             BOOL res = RemoveDirectoryA(cpath);
             free(cpath);
             return res != 0;
        }
        free(cpath);
        return false;
    }

    bool success = true;
    usize original_len = pb->len;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
            continue;
        }

        if (!el_pathbuf_join(pb, el_sv_from_cstr(fd.cFileName))) {
            success = false;
            break;
        }

        if (!rm_recursive_internal(pb)) {
            success = false;
        }

        pb->len = original_len;
        if (!success) break;
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    if (success) {
        success = (RemoveDirectoryA(cpath) != 0);
    }

    free(cpath);
    return success;
}

bool el_fs_rm_recursive(ElPathView path) {
    if (path.len == 0) return false;
    ElPathBuf pb;
    if (!el_pathbuf_init_from(&pb, path)) return false;
    bool res = rm_recursive_internal(&pb);
    el_pathbuf_destroy(&pb);
    return res;
}

bool el_fs_file_exists(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    DWORD attrs = GetFileAttributesA(cpath);
    free(cpath);
    return (attrs != INVALID_FILE_ATTRIBUTES);
}

ElFileType el_fs_get_file_type(ElPathView path) {
    if (path.len == 0) return EL_FILE_NOT_FOUND;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return EL_FILE_NOT_FOUND;

    DWORD attrs = GetFileAttributesA(cpath);
    free(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) return EL_FILE_NOT_FOUND;
    if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) return EL_FILE_SYMLINK;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) return EL_FILE_DIR;
    return EL_FILE_REGULAR;
}

bool el_fs_set_executable(ElPathView path, bool enabled) {
    (void)path;
    (void)enabled;
    // on Windows, execution depends on file extension, not file attributes.
    return true;
}

#endif // EL_PLATFORM_IS_WINDOWS

