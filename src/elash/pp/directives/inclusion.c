#include "../preproc-internals.h"

#include <elash/util/pathview.h>
#include <elash/util/pathbuf.h>
#include <elash/util/dynarena.h>
#include <elash/lexer/lexer.h>
#include <elash/source/doc.h>

#include <elash/util/strbuf.h>
#include <elash/util/todo.h>
#include <elash/util/fs.h>

static bool is_valid_path_token(ElTokenType type) {
    return type == EL_TT_IDENT || type == EL_TT_STRING_LITERAL || type == EL_TT_MINUS
        || type == EL_TT_INT_LITERAL || type == EL_TT_SLASH || type == EL_TT_DOT;
}

static bool parse_inc_path(ElPreproc* pp, ElPpIncPath* out_path) {
    ElToken first_tok;
    if (!_el_pp_read(pp, &first_tok)) return false;

    const uint directive_line = first_tok.span.ranges[0].start.line;

    ElToken next;
    ElStringBuf path_sb;
    el_strbuf_init(&path_sb);

    ElToken last_tok = first_tok;

    if (_el_pp_peek(pp, &next) && next.type == EL_TT_COLON) {
        _el_pp_read(pp, &next); // ':'
        out_path->scope = el_dynarena_clone_sv(pp->iarena, first_tok.lexeme);
    } else {
        out_path->scope = EL_SV_NULL;
        el_strbuf_append(&path_sb, first_tok.lexeme);
    }

    while (
        _el_pp_peek(pp, &next) &&
        is_valid_path_token(next.type) &&
        next.span.ranges[0].start.line == directive_line
    ) {
        _el_pp_read(pp, &next);
        el_strbuf_append(&path_sb, next.lexeme);
        last_tok = next;
    }

    out_path->ipath = el_dynarena_clone_sv(pp->iarena, el_strbuf_view(&path_sb));
    out_path->span  = el_srcspan_merge(first_tok.span, last_tok.span);
    el_strbuf_destroy(&path_sb);

    return true;
}

static bool _el_pp_resolve_inc_path(ElPreproc* pp, const ElPpIncPath* path, ElPpIncFile* out_file) {
    ElStringView base_path = EL_SV_NULL;
    bool is_system = false;

    if (el_sv_is_null(path->scope)) {
        if (!pp->frame || !pp->frame->doc) return false;
        base_path = el_pathview_dirname(pp->frame->doc->filename);
    } else {
        ElPpIncMapping* mapping = pp->imap->head;
        while (mapping != NULL) {
            if (el_sv_eql(mapping->name, path->scope)) {
                base_path = mapping->path;
                is_system = mapping->is_system;
                break;
            }
            mapping = mapping->next;
        }

        if (el_sv_is_null(base_path)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.include-map",
                path->span, "include mapping not found: ${scope}",
                EL_DIAG_STRING("scope", path->scope),
            );
        }
    }

    ElPathBuf full_path;
    el_pathbuf_init(&full_path);
    el_pathbuf_join(&full_path, base_path);
    el_pathbuf_join(&full_path, path->ipath);

    ElPathView pv = el_pathbuf_view(&full_path);
    bool exists = el_fs_file_exists(pv);

    if (!exists && el_pathview_ext(path->ipath).len == 0) {
        el_strbuf_append_cstr(&full_path, ".eh");
        pv = el_pathbuf_view(&full_path);
        exists = el_fs_file_exists(pv);
    }

    if (exists) {
        out_file->is_system = is_system;
        out_file->path = el_dynarena_clone_sv(pp->iarena, pv);
        el_pathbuf_destroy(&full_path);
        return true;
    }

    el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.include-not-found",
        path->span, "include file not found: ${path}",
        EL_DIAG_STRING("path", pv),
    );
    el_pathbuf_destroy(&full_path);
    return false;
}

bool _el_pp_handle_include(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->include_depth >= INCLUDE_DEPTH_LIMIT) {
        el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.include-depth",
            dspan, "include depth limit exceeded",
        );
        el_diag_help(
            pp->diag, "check if you don't have any recursive includes without a stop condition"
        );
        el_diag_help(
            pp->diag, "include depth limit is set to ${limit}",
            EL_DIAG_INT("limit", INCLUDE_DEPTH_LIMIT)
        );
        return false;
    }

    ElPpIncPath path;
    if (!parse_inc_path(pp, &path)) return false;

    ElPpIncFile file;
    if (!_el_pp_resolve_inc_path(pp, &path, &file)) return false;

    ElSourceDocument* doc = EL_DYNARENA_NEW(pp->iarena, ElSourceDocument);
    if (el_srcdoc_init_from_file(doc, el_dynarena_make_cstr(pp->iarena, file.path)) != 0) return false;

    ElLexer* lexer = EL_DYNARENA_NEW(pp->iarena, ElLexer);
    el_lexer_init(lexer, doc, EL_LEXER_FLAGS_DEFAULT);

    _el_pp_push_frame(pp, el_lexer_as_token_stream(lexer), doc);
    return true;
}

bool _el_pp_skip_include(ElPreproc* pp) {
    ElPpIncPath path;
    return parse_inc_path(pp, &path);
}

bool _el_pp_handle_embed(ElPreproc* pp, ElSourceSpan dspan) {
    (void) pp, (void) dspan;
    EL_TODO("implement #embed");
}

bool _el_pp_skip_embed(ElPreproc* pp) {
    (void) pp;
    EL_TODO("implement #embed");
}
