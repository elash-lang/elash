#pragma once

#include <elash/defs/sv.h>

bool el_string_to_u64(ElStringView str, uint base, uint64_t* out);
bool el_string_to_i64(ElStringView str, uint base, int64_t* out);

bool el_string_to_long_double(ElStringView str, long double* out);
