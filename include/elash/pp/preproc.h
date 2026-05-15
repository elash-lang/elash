#pragma once

#include <elash/util/dynarena.h>

#include <elash/lexer/tokbuf.h>
#include <elash/lexer/tokque.h>
#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

#include <elash/pp/error.h>
#include <elash/pp/vars.h>

#include <stdbool.h>

typedef struct ElPreprocessor {
    ElTokenStream input;
    ElTokenQueue pending;

    ElDynArena arena;

    ElPpVars vars;

    ElPpErrorDetails last_err_details;
} ElPreprocessor;

ElPpErrorCode el_pp_init(ElPreprocessor* pp, ElTokenStream input);
void          el_pp_destroy(ElPreprocessor* pp);

ElPpErrorCode el_pp_reset(ElPreprocessor* pp, ElTokenStream input);

ElPpErrorCode el_pp_next(ElPreprocessor* pp, ElToken* out_tok, ElDiagEngine* engine);

ElTokenStream el_pp_as_token_stream(ElPreprocessor* pp);

////// IMPLEMENTATION DETAILS //////////////////////
// note: those functions are implementation
// details and should not be called directly
// outside internal el-pp implementation.
ElPpErrorCode _el_pp_ret_error(ElPreprocessor* pp, ElPpErrorCode code);
ElPpErrorCode _el_pp_ret_success(ElPreprocessor* pp);
////////////////////////////////////////////////////
