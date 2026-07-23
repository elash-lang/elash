#include <elash/srcdoc/srcdoc.h>

#include <elash/util/strbuf.h>
#include <elash/lexer/token.h>

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

ElSrcDocErrorCode _el_strdoc_ret_err(bool result) {
    if (!result) return EL_SRCDOC_ERR_ALLOC_FAILED;
    return EL_SRCDOC_ERR_SUCCESS;
}

bool _el_strdoc_get_file_size(FILE* f, usize* out_size) {
    if (fseek(f, 0, SEEK_END) != 0) {
        return false;
    }
    long signed_size = ftell(f);
    if (signed_size == -1L) {
        return false;
    }

    if (signed_size < 0 || (usize)signed_size > SIZE_MAX) {
        return false;
    }
    *out_size = (usize)signed_size;

    if (fseek(f, 0, SEEK_SET) != 0)
        return false;

    return true;
}

ElSrcDocErrorCode el_srcdoc_init_empty(ElSourceDocument* srcdoc, ElStringView filename) {
    srcdoc->filename = filename;
    return _el_strdoc_ret_err(el_strbuf_init(&srcdoc->content));
}
ElSrcDocErrorCode el_srcdoc_init_from_str(ElSourceDocument* srcdoc, ElStringView sv, ElStringView filename) {
    srcdoc->filename = filename;
    return _el_strdoc_ret_err(el_strbuf_init_from(&srcdoc->content, sv));
}

ElSrcDocErrorCode el_srcdoc_init_from_file(ElSourceDocument* srcdoc, const char* path) {
    ElSrcDocErrorCode err = EL_SRCDOC_ERR_SUCCESS;
    srcdoc->filename = el_sv_from_cstr(path);

    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        err = EL_SRCDOC_ERR_FOPEN_FAILED;
        goto fail;
    }

    usize size;
    if (!_el_strdoc_get_file_size(f, &size)) {
        err = EL_SRCDOC_ERR_FTELL_FAILED;
        goto fail;
    }

    if (!el_strbuf_init_with_cap(&srcdoc->content, size)) {
        err = EL_SRCDOC_ERR_ALLOC_FAILED;
        goto fail;
    }

    if (!el_strbuf_resize(&srcdoc->content, size)) {
        err = EL_SRCDOC_ERR_ALLOC_FAILED;
        el_strbuf_destroy(&srcdoc->content);
        goto fail;
    }

    usize readed = fread(srcdoc->content.data, 1, size, f);

    if (readed != size) {
        err = EL_SRCDOC_ERR_FREAD_FAILED;
        el_strbuf_destroy(&srcdoc->content);
        goto fail;
    }

    goto end;

fail:
    if (f != NULL) {
        fclose(f);
    }
end:
    return err;
}

ElSrcDocErrorCode el_srcdoc_copy(const ElSourceDocument* src, ElSourceDocument* dst) {
    return _el_strdoc_ret_err(el_srcdoc_init_from_strbuf(dst, &src->content, src->filename));
}
void el_srcdoc_move(ElSourceDocument* src, ElSourceDocument* dst) {
    return el_srcdoc_init_from_strbuf_move(dst, &src->content, src->filename);
}

ElSrcDocErrorCode el_srcdoc_init_from_strbuf(ElSourceDocument* srcdoc, const ElStringBuf* buf, ElStringView filename) {
    srcdoc->filename = filename;
    return _el_strdoc_ret_err(el_strbuf_copy(buf, &srcdoc->content));
}
void el_srcdoc_init_from_strbuf_move(ElSourceDocument* srcdoc, ElStringBuf* buf, ElStringView filename) {
    srcdoc->filename = filename;
    return el_strbuf_move(buf, &srcdoc->content);
}

void el_srcdoc_destroy(ElSourceDocument* srcdoc) {
    el_strbuf_destroy(&srcdoc->content);
}
void el_srcdoc_clear(ElSourceDocument* srcdoc) {
    el_strbuf_clear(&srcdoc->content);
}

ElSrcDocErrorCode el_srcdoc_append_token(ElSourceDocument* srcdoc, const ElToken* tok) {
    if (!el_token_to_raw_string(tok, &srcdoc->content)) {
        return EL_SRCDOC_ERR_ALLOC_FAILED;
    }
    return EL_SRCDOC_ERR_SUCCESS;
}

ElSrcDocErrorCode el_srcdoc_append_str(ElSourceDocument* srcdoc, ElStringView sv) {
    return _el_strdoc_ret_err(el_strbuf_append(&srcdoc->content, sv));
}

ElSrcDocErrorCode el_srcdoc_concat(const ElSourceDocument* src1,
                                   const ElSourceDocument* src2,
                                   ElSourceDocument* dst,
                                   ElStringView filename)
{
    ElSrcDocErrorCode err = el_srcdoc_init_empty(dst, filename);
    if (err != EL_SRCDOC_ERR_SUCCESS) { return err; }

    if (!el_strbuf_append_buf(&dst->content, &src1->content)) {
        return EL_SRCDOC_ERR_ALLOC_FAILED;
    }
    if (!el_strbuf_append_buf(&dst->content, &src2->content)) {
        return EL_SRCDOC_ERR_ALLOC_FAILED;
    }
    return EL_SRCDOC_ERR_SUCCESS;
}

ElStringView el_srcdoc_content(const ElSourceDocument* srcdoc) {
    return el_strbuf_view(&srcdoc->content);
}

usize el_srcdoc_length(const ElSourceDocument* srcdoc) {
    return srcdoc->content.len;
}

ElSrcDocErrorCode el_srcdoc_print(const ElSourceDocument* srcdoc, FILE* out) {
    if (el_sv_print(el_srcdoc_content(srcdoc), out) != srcdoc->content.len) {
        return EL_SRCDOC_ERR_FWRITE_FAILED;
    }
    return EL_SRCDOC_ERR_SUCCESS;
}
