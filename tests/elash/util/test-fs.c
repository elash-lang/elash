#include <criterion/criterion.h>

#include <elash/util/pathview.h>
#include <elash/util/fs.h>

#define TEST_DIR(NAME) "test-tmp-fs-" NAME

// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is the worst linter i've used in my entire life
static void setupdir(const char* dir) {
    // oh no two lines, so much cognitive complexity!
    el_fs_rm_recursive(el_sv_from_cstr(dir));
    cr_assert(el_fs_mkdir_all(el_sv_from_cstr(dir)));
}
static void cleanupdir(const char* dir) {
    cr_assert(el_fs_rm_recursive(el_sv_from_cstr(dir)));
}

TestSuite(el_fs);

Test(el_fs, mkdir_rm) {
    const char* root = TEST_DIR("mkdir-rm");
    setupdir(root);

    ElPathView path = EL_PV(TEST_DIR("mkdir-rm") "/dir1");
    cr_assert(!el_fs_file_exists(path));

    cr_assert(el_fs_mkdir_all(path));
    cr_assert(el_fs_file_exists(path));
    cr_assert_eq(el_fs_get_file_type(path), EL_FILE_DIR);

    cr_assert(el_fs_rm(path));
    cr_assert(!el_fs_file_exists(path));

    cleanupdir(root);
}

Test(el_fs, mkdir_all) {
    const char* root = TEST_DIR("mkdir-all");
    setupdir(root);

    ElPathView path = EL_PV(TEST_DIR("mkdir-all") "/a/b/c");
    cr_assert(!el_fs_file_exists(path));

    cr_assert(el_fs_mkdir_all(path));
    cr_assert(el_fs_file_exists(path));
    cr_assert_eq(el_fs_get_file_type(EL_PV(TEST_DIR("mkdir-all") "/a")), EL_FILE_DIR);
    cr_assert_eq(el_fs_get_file_type(EL_PV(TEST_DIR("mkdir-all") "/a/b")), EL_FILE_DIR);
    cr_assert_eq(el_fs_get_file_type(EL_PV(TEST_DIR("mkdir-all") "/a/b/c")), EL_FILE_DIR);

    cleanupdir(root);
}

Test(el_fs, mkfile) {
    const char* root = TEST_DIR("mkfile");
    setupdir(root);

    ElPathView path = EL_PV(TEST_DIR("mkfile") "/file.txt");
    cr_assert(!el_fs_file_exists(path));

    cr_assert(el_fs_mkfile(path));
    cr_assert(el_fs_file_exists(path));
    cr_assert_eq(el_fs_get_file_type(path), EL_FILE_REGULAR);

    cr_assert(el_fs_mkfile_if_not_exists(path));

    cr_assert(el_fs_rm(path));
    cr_assert(!el_fs_file_exists(path));

    cleanupdir(root);
}

Test(el_fs, read_write_file) {
    const char* root = TEST_DIR("rw-file");
    setupdir(root);

    ElPathView path = EL_PV(TEST_DIR("rw-file") "/file.txt");
    ElStringView content = EL_SV("hello world\nthis is a test");

    cr_assert(el_fs_write_file(path, content));
    cr_assert(el_fs_file_exists(path));

    ElStringBuf read_buf;
    el_strbuf_init(&read_buf);

    cr_assert(el_fs_read_file(path, &read_buf));
    cr_assert(el_strbuf_eql_to(&read_buf, content));

    el_strbuf_destroy(&read_buf);
    cleanupdir(root);
}

Test(el_fs, rm_recursive) {
    const char* root = TEST_DIR("rm-recursive");
    setupdir(root);

    el_fs_mkdir_all(EL_PV(TEST_DIR("rm-recursive") "/deep/dir/structure"));
    el_fs_mkfile(EL_PV(TEST_DIR("rm-recursive") "/deep/file1"));
    el_fs_mkfile(EL_PV(TEST_DIR("rm-recursive") "/deep/dir/file2"));

    cr_assert(el_fs_file_exists(EL_PV(TEST_DIR("rm-recursive") "/deep/dir/structure")));

    cr_assert(el_fs_rm_recursive(EL_PV(TEST_DIR("rm-recursive") "/deep")));
    cr_assert(!el_fs_file_exists(EL_PV(TEST_DIR("rm-recursive") "/deep")));

    cleanupdir(root);
}

Test(el_fs, set_executable) {
#if EL_PLATFORM_IS_POSIX
    const char* root = TEST_DIR("set-executable");
    setupdir(root);

    ElPathView path = EL_PV(TEST_DIR("set-executable") "/script.sh");
    el_fs_mkfile(path);

    cr_assert(el_fs_set_executable(path, true));
    cr_assert(el_fs_set_executable(path, false));

    cleanupdir(root);
#endif
}

Test(el_fs, path_abs) {
    ElPathBuf abs;
    el_pathbuf_init(&abs);

    ElPathView rel = EL_PV(".");
    cr_assert(el_fs_path_abs(rel, &abs));
    cr_assert(el_pathview_is_absolute(el_pathbuf_view(&abs)));

    el_pathbuf_clear(&abs);
    rel = EL_PV("test-file-that-does-not-exist");
    cr_assert(el_fs_path_abs(rel, &abs));
    cr_assert(el_pathview_is_absolute(el_pathbuf_view(&abs)));

    el_pathbuf_clear(&abs);
    rel = EL_PV("include");
    cr_assert(el_fs_path_abs(rel, &abs));
    ElPathView abs_view = el_pathbuf_view(&abs);
    cr_assert(el_pathview_is_absolute(abs_view));
    cr_assert(el_sv_ends_with(abs_view, EL_SV("include")));

    el_pathbuf_destroy(&abs);
}
