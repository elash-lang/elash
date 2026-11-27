#pragma once

#include <elash/util/strbuf.h>
#include <elash/defs/attr.h>
#include <elash/defs/sv.h>

#include <stdio.h>
#include <stdarg.h>

typedef struct ElWriter ElWriter;

typedef void ElWriteFn(ElWriter* self, ElStringView content);
typedef void ElWriteCStrFn(ElWriter* self, const char* content);
typedef void ElWriteCharFn(ElWriter* self, char content);
typedef void ElWritefFn(ElWriter* self, const char* fmt, va_list args);

typedef struct ElWriterVt {
    ElWriteFn*     write;      // optional: defaults to write_char loop over all characters
    ElWritefFn*    writef;     // optional: defaults to formatting and calling write
    ElWriteCStrFn* write_cstr; // optional: defaults to write(el_sv_from_cstr(content))
    ElWriteCharFn* write_char; // required
} ElWriterVt;

extern const ElWriterVt el_sb_writer_vt;
extern const ElWriterVt el_file_writer_vt;

typedef struct ElWriter {
    const ElWriterVt* vt;
    void* data;
} ElWriter;

void el_writevf(ElWriter* writer, const char* fmt, va_list args);

void el_writef(ElWriter* writer, const char* fmt, ...)
    EL_ATTR_PRINTF_LIKE(2, 3);

void el_write(ElWriter* writer, ElStringView content);
void el_write_cstr(ElWriter* writer, const char* content);
void el_write_char(ElWriter* writer, char content);

#define el_swrite(writer, content) \
    el_write(writer, EL_SV(content))

ElWriter el_file_writer(FILE* file);
ElWriter el_sb_writer(ElStringBuf* sb);
