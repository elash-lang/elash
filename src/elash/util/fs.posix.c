#include <elash/defs/platform.h>

#if EL_PLATFORM_IS_POSIX

#define _GNU_SOURCE

#include <elash/util/fs.h>
#include <elash/util/pathview.h>
#include <elash/util/pathbuf.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>

#if EL_PLATFORM_IS_MACOS
#include <mach-o/dyld.h>
#endif

#define DEFAULT_FILE_PERMS 0644
#define DEFAULT_DIR_PERMS  0755

bool el_fs_path_abs(ElPathView path, ElPathBuf* out_abs) {
    if (path.len == 0) return false;

    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    char* abs_path = realpath(cpath, NULL);
    if (abs_path != NULL) {
        el_strbuf_clear(out_abs);
        bool success = el_strbuf_append(out_abs, el_sv_from_cstr(abs_path));
        free(abs_path);
        free(cpath);
        return success;
    }

    if (el_pathview_is_absolute(path)) {
        el_strbuf_clear(out_abs);
        bool success = el_strbuf_append(out_abs, path);
        free(cpath);
        return success;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        free(cpath);
        return false;
    }

    el_strbuf_clear(out_abs);
    if (!el_strbuf_append(out_abs, el_sv_from_cstr(cwd))) {
        free(cpath);
        return false;
    }
    bool success = el_pathbuf_join(out_abs, path);
    free(cpath);
    return success;
}

bool el_fs_read_file(ElPathView path, ElStringBuf* out) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int fd = open(cpath, O_RDONLY);
    free(cpath);
    if (fd == -1) return false;

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return false;
    }

    usize size = (usize)st.st_size;
    if (!el_strbuf_resize(out, size)) {
        close(fd);
        return false;
    }

    ssize_t nread = read(fd, out->data, size);
    close(fd);

    if (nread == -1 || (usize)nread != size) {
        return false;
    }

    return true;
}

bool el_fs_write_file(ElPathView path, ElStringView content) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int fd = open(cpath, O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_FILE_PERMS);
    free(cpath);
    if (fd == -1) return false;

    ssize_t nwritten = write(fd, content.data, content.len);
    close(fd);

    return nwritten != -1 && (usize)nwritten == content.len;
}

bool el_fs_mkdir(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int res = mkdir(cpath, DEFAULT_DIR_PERMS);
    if (res == 0) {
        free(cpath);
        return true;
    }

    if (errno == EEXIST) {
        struct stat st;
        if (stat(cpath, &st) == 0 && S_ISDIR(st.st_mode)) {
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

    int fd = open(cpath, O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_FILE_PERMS);
    if (fd == -1) {
        free(cpath);
        return false;
    }

    close(fd);
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

    int fd = open(cpath, O_CREAT | O_WRONLY | O_EXCL, DEFAULT_FILE_PERMS);
    if (fd == -1) {
        if (errno == EEXIST) {
            free(cpath);
            return true;
        }
        free(cpath);
        return false;
    }

    close(fd);
    free(cpath);
    return true;
}

bool el_fs_touch(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int res = utime(cpath, NULL);
    if (res == -1 && errno == ENOENT) {
        int fd = open(cpath, O_CREAT | O_WRONLY, DEFAULT_FILE_PERMS);
        if (fd != -1) {
            close(fd);
            res = 0;
        }
    }

    free(cpath);
    return res == 0;
}

bool el_fs_rm(ElPathView path) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int res = remove(cpath);
    free(cpath);
    return res == 0;
}

static bool rm_recursive_internal(ElPathBuf* pb) {
    ElPathView current_pv = el_pathbuf_view(pb);
    char* cpath = el_sv_to_cstr_alloc(current_pv);
    if (cpath == NULL) return false;

    if (unlink(cpath) == 0) {
        free(cpath);
        return true;
    }

    if (errno != EISDIR && errno != EPERM) {
        free(cpath);
        return false;
    }

    DIR* dir = opendir(cpath);
    free(cpath);
    if (!dir) return false;

    bool success = true;
    struct dirent* entry;
    usize original_len = pb->len;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!el_pathbuf_join(pb, el_sv_from_cstr(entry->d_name))) {
            success = false;
            break;
        }

        if (!rm_recursive_internal(pb)) {
            success = false;
        }

        pb->len = original_len;
        if (!success) break;
    }

    closedir(dir);
    if (success) {
        char* dpath = el_sv_to_cstr_alloc(el_pathbuf_view(pb));
        if (dpath != NULL) {
            success = (rmdir(dpath) == 0);
            free(dpath);
        } else {
            success = false;
        }
    }
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

    struct stat st;
    int res = stat(cpath, &st);
    free(cpath);
    return res == 0;
}

ElFileType el_fs_get_file_type(ElPathView path) {
    if (path.len == 0) return EL_FILE_NOT_FOUND;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return EL_FILE_NOT_FOUND;

    struct stat st;
    if (lstat(cpath, &st) != 0) {
        free(cpath);
        return EL_FILE_NOT_FOUND;
    }

    free(cpath);
    if (S_ISREG(st.st_mode)) return EL_FILE_REGULAR;
    if (S_ISDIR(st.st_mode)) return EL_FILE_DIR;
    if (S_ISLNK(st.st_mode)) return EL_FILE_SYMLINK;
    return EL_FILE_OTHER;
}

bool el_fs_set_executable(ElPathView path, bool enabled) {
    if (path.len == 0) return false;
    char* cpath = el_sv_to_cstr_alloc(path);
    if (cpath == NULL) return false;

    int fd = open(cpath, O_RDONLY);
    free(cpath);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return false;
    }

    mode_t mode = st.st_mode;
    if (enabled) {
        mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    } else {
        mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
    }

    int res = fchmod(fd, mode);
    close(fd);
    return res == 0;
}

#endif // EL_PLATFORM_IS_POSIX

