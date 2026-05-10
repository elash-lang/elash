#pragma once

#include <elc/codegen/backend.h>
#include <elash/util/dynarena.h>

ElcCodegenBackend elc_make_llvm_codegen(ElDynArena* arena);
