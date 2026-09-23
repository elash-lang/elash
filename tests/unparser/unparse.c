#include <elash/defs/int-types.h>
#include <elash/util/dynarena.h>

#include <elash/ast/equal.h>
#include <elash/diag/engine.h>
#include <elash/source/doc.h>
#include <elash/pp/preproc.h>
#include <elash/lexer/lexer.h>
#include <elash/lexer/tokbuf.h>
#include <elash/parser/parser.h>
#include <elash/unparser/unparser.h>

#include <stdio.h>

#include <elash/ast/dump/module.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.eu>\n", argv[0]);
        return 1;
    }

    ElDynArena arena;
    el_dynarena_init(&arena);

    ElTkBufStream stream;
    ElDiagEngine diag;
    ElPpIncMap imap = {0};

    el_diag_engine_init(&diag, &arena);

    ElTokenBuf toks;
    el_tkbuf_init(&toks);

    ElSourceDocument doc;
    if (el_srcdoc_init_from_file(&doc, argv[1]) != EL_SRCDOC_ERR_SUCCESS) {
        fprintf(stderr, "Failed to load: %s\n", argv[1]);
        return 1;
    }

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    ElPreproc pp;
    el_pp_init(&pp, el_lexer_as_token_stream(&lexer), &doc, &arena, &imap, NULL);

    ElParser parser;
    el_parser_init(&parser, el_pp_as_token_stream(&pp), &diag, &arena, &arena, NULL);
    ElAstModule* orig = el_parse_module(&parser);
    el_parser_destroy(&parser);

    ElUnparser unparser;
    el_unparser_init(&unparser, &toks, &arena);
    el_unparser_unparse_module(&unparser, orig);

    el_parser_init(&parser, el_tkbuf_as_stream(&stream, &toks), &diag, &arena, &arena, NULL);
    ElAstModule* repro = el_parse_module(&parser);
    el_parser_destroy(&parser);

    // this is ignored anyway if the test passes,
    // so let's print it for debugging purposes
    fputs("orig:\n", stderr);
    el_ast_dump_module(orig, 2, stderr);
    fputs("\n", stderr);
    fputs("repro:\n", stderr);
    el_ast_dump_module(repro, 2, stderr);

    int result = el_ast_equal_module(orig, repro) ? 0 : 1;

    el_pp_free(&pp);
    el_dynarena_free(&arena);
    return result;
}
