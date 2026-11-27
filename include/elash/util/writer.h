#pragma once

#include <elash/util/strbuf.h>
#include <elash/defs/sv.h>

#include <stdio.h>

typedef struct ElWriter ElWriter;

typedef void ElWriteFn(ElWriter* self, ElStringView content);
typedef void ElWriteCStrFn(ElWriter* self, const char* content);
typedef void ElWriteCharFn(ElWriter* self, char content);

typedef struct ElWriterVt {
    ElWriteFn*     write;      // optional: defaults to write_char loop over all characters
    ElWriteCStrFn* write_cstr; // optional: defaults to write(el_sv_from_cstr(content))
    ElWriteCharFn* write_char; // required
} ElWriterVt;

extern const ElWriterVt el_sb_writer_vt;
extern const ElWriterVt el_file_writer_vt;

typedef struct ElWriter {
    const ElWriterVt* vt;
    void* data;
} ElWriter;

void el_write(ElWriter* writer, ElStringView content);
void el_write_cstr(ElWriter* writer, const char* content);
void el_write_char(ElWriter* writer, char content);

ElWriter el_file_writer(FILE* file);
ElWriter el_sb_writer(ElStringBuf* sb);
