#include <elash/util/fs.h>
#include <elash/util/pathview.h>
#include <elash/util/pathbuf.h>

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
