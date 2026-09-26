#pragma once

#include <elash/lexer/token.h>
#include <elash/util/strbuf.h>

#include <elash/source/error.h>

#include <stdio.h>

typedef struct ElSourceDocument {
    ElStringBuf content;
    ElStringView filename;
    bool is_system;
} ElSourceDocument;

/// Creates an empty source document with given file name.
void el_srcdoc_init_empty(ElSourceDocument* srcdoc, ElStringView filename);
/// Creates source document with the provided content and file name.
void el_srcdoc_init_from_str(ElSourceDocument* srcdoc, ElStringView sv, ElStringView filename);
/// Creates a source document by reading content from the file at the given path.
ElSrcDocStatus el_srcdoc_init_from_file(ElSourceDocument* srcdoc, const char* path);
/// Creates a source document by copying content from the provided string buffer.
void el_srcdoc_init_from_strbuf(ElSourceDocument* srcdoc, const ElStringBuf* buf, ElStringView filename);
/// Creates a source document by taking ownership of the content from the provided string buffer.
void el_srcdoc_init_from_strbuf_move(ElSourceDocument* srcdoc, ElStringBuf* buf, ElStringView filename);

/// Creates a deep copy of the source document.
void el_srcdoc_copy(const ElSourceDocument* src, ElSourceDocument* dst);
/// Moves the content from the source document to the destination.
void el_srcdoc_move(ElSourceDocument* src, ElSourceDocument* dst);

/// Frees the resources associated with the source document.
void el_srcdoc_free(ElSourceDocument* srcdoc);
/// Clears the content of the source document.
void el_srcdoc_clear(ElSourceDocument* srcdoc);

/// Appends the raw text form of the token to the source document.
void el_srcdoc_append_token(ElSourceDocument* srcdoc, const ElToken* tok);
/// Appends a string view to the source document.
void el_srcdoc_append_str(ElSourceDocument* srcdoc, ElStringView sv);

/// Concatenates two source documents into a new one.
void el_srcdoc_concat(
    const ElSourceDocument* src1, const ElSourceDocument* src2,
    ElSourceDocument* dst, ElStringView filename
);

/// Returns a string view of the document content.
ElStringView el_srcdoc_content(const ElSourceDocument* srcdoc);
/// Returns the length of the document content.
usize el_srcdoc_length(const ElSourceDocument* srcdoc);
/// Prints the document content to the specified file stream.
ElSrcDocStatus el_srcdoc_print(const ElSourceDocument* srcdoc, FILE* out);
