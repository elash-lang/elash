#pragma once

#include <elash/util/strbuf.h>
#include <elash/diag/meta.h>

void el_diag_render_template(ElStringView template, const ElDiagMeta* meta, ElStringBuf* out);
