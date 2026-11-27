#include <elash/util/writer.h>
#include <elash/util/assert.h>
#include <elash/defs/sv.h>

void el_write(ElWriter* writer, ElStringView content) {
    EL_ASSERT(writer != NULL && writer->vt != NULL,
              "should not be null");

    if (writer->vt->write != NULL) {
        return writer->vt->write(writer, content);
    }

    EL_ASSERT(writer->vt->write_char != NULL,
              "write_char implementation is required");

    for (usize i = 0; i < content.len; ++i) {
        writer->vt->write_char(writer, content.data[i]);
    }
}

void el_write_cstr(ElWriter* writer, const char* content) {
    EL_ASSERT(writer != NULL && writer->vt != NULL,
              "should not be null");

    if (writer->vt->write_cstr != NULL) {
        return writer->vt->write_cstr(writer, content);
    }

    return el_write(writer, el_sv_from_cstr(content));
}

void el_write_char(ElWriter* writer, char content) {
    EL_ASSERT(writer != NULL && writer->vt != NULL,
              "should not be null");

    EL_ASSERT(writer->vt->write_char != NULL,
              "write_char implementation is required");

    return writer->vt->write_char(writer, content);
}

ElWriter el_sb_writer(ElStringBuf* sb) {
    EL_ASSERT(sb != NULL, "should not be null");
    return (ElWriter) {
        .data = sb,
        .vt = &el_sb_writer_vt,
    };
}

ElWriter el_file_writer(FILE* file) {
    EL_ASSERT(file != NULL, "should not be null");
    return (ElWriter) {
        .data = file,
        .vt = &el_file_writer_vt,
    };
}

static void sb_write(ElWriter* self, ElStringView content)     { el_strbuf_append((ElStringBuf*)self->data, content);      }
static void sb_write_cstr(ElWriter* self, const char* content) { el_strbuf_append_cstr((ElStringBuf*)self->data, content); }
static void sb_write_char(ElWriter* self, char content)        { el_strbuf_append_char((ElStringBuf*)self->data, content); }

static void file_write(ElWriter* self, ElStringView content)     { el_sv_print(content, (FILE*)self->data); }
static void file_write_cstr(ElWriter* self, const char* content) { fputs(content, (FILE*)self->data);       }
static void file_write_char(ElWriter* self, char content)        { fputc(content, (FILE*)self->data);       }

const ElWriterVt el_sb_writer_vt = {
    .write = sb_write,
    .write_cstr = sb_write_cstr,
    .write_char = sb_write_char,
};

const ElWriterVt el_file_writer_vt = {
    .write = file_write,
    .write_cstr = file_write_cstr,
    .write_char = file_write_char,
};
