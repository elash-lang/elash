#include <elash/util/strconv.h>
#include <elash/util/assert.h>
#include <stdint.h>

#define MIN_BASE 2
#define MAX_BASE 32

ElUint128 el_string_to_u128(ElDiagEngine* engine, ElStringView str, uint base, ElSourceSpan span) {
    EL_ASSERT(str.len != 0, "empty string passed to el_string_to_u128");
    EL_ASSERT(base >= MIN_BASE && base <= MAX_BASE, "invalid base range");

    ElUint128 res = EL_UINT128(0);
    ElUint128 base128 = EL_UINT128(base);
    bool has_digits = false;
    for (usize i = 0; i < str.len; i++) {
        char c = str.data[i];
        int digit = -1;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10; // NOLINT(readability-magic-numbers)
        } else if (c >= 'A' && c <= 'Z') {
            digit = c - 'A' + 10; // NOLINT(readability-magic-numbers)
        }

        if (digit >= 0 && digit < (int) base) {
            ElUint128 digit128 = EL_UINT128(digit);
            ElUint128 max_val = UINT128_MAX;
            ElUint128 threshold = el_u128_div(el_u128_sub(max_val, digit128), base128);

            if (el_u128_gt(res, threshold)) {
                el_diag_report(
                    engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                    "literal value exceeds maximum representable value"
                );
                return EL_UINT128(0);
            }
            res = el_u128_add(el_u128_mul(res, base128), digit128);
            has_digits = true;
        } else if (c == '\'') {
           continue;
        } else {
            el_diag_report(
                engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                "character '${char}' is not a valid digit for base ${base}",
                EL_DIAG_CHAR("char", c),
                EL_DIAG_INT("base", (int)base),
            );
            return EL_UINT128(0);
        }
    }

    EL_ASSERT(has_digits, "should have digits");
    return res;
}

ElInt128 el_string_to_i128(ElDiagEngine* engine, ElStringView str, uint base, ElSourceSpan span) {
    EL_ASSERT(str.len != 0, "empty string passed to el_string_to_i128");
    EL_ASSERT(base >= MIN_BASE && base <= MAX_BASE, "invalid base range");

    bool negative = false;
    usize start = 0;

    if (str.data[0] == '-') {
        negative = true;
        start = 1;
    } else if (str.data[0] == '+') {
        start = 1;
    }

    if (start >= str.len) {
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
            "numeric literal consists only of a sign; digits are required"
        );
        return EL_INT128(0);
    }

    ElStringView absolute = { .data = str.data + start, .len = str.len - start };
    ElUint128 ures = el_string_to_u128(engine, absolute, base, span);
    if (el_diag_engine_has_errors(engine)) return EL_INT128(0);

    if (negative) {
        ElUint128 min_abs = el_i128_abs_u128(INT128_MIN);

        if (el_u128_gt(ures, min_abs)) {
            el_diag_report(
                engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                "literal value exceeds minimum representable value for 128-bit signed integer"
            );
            return EL_INT128(0);
        }

        if (el_u128_eq(ures, min_abs)) {
            return INT128_MIN;
        } else {
            return el_i128_neg(el_u128_bitcast_i128(ures));
        }
    } else {
        ElUint128 max_val = el_i128_bitcast_u128(INT128_MAX);
        if (el_u128_gt(ures, max_val)) {
            el_diag_report(
                engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                "literal value exceeds maximum representable value for 128-bit signed integer"
            );
            return EL_INT128(0);
        }
        return el_u128_bitcast_i128(ures);
    }
}

static inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
static inline bool is_sep(char c) { return c == '\''; }

static bool parse_int_part(ElDiagEngine* engine, ElSourceSpan span, ElStringView str, usize* i, double* res, bool* has_digits) {
    bool prev_digit = false;
    while (*i < str.len) {
        char c = str.data[*i];
        if (is_digit(c)) {
            *res = (*res * 10.0) + (c - '0'); // NOLINT(readability-magic-numbers)
            *has_digits = true;
            prev_digit = true;
            (*i)++;
        } else if (is_sep(c)) {
            if (!prev_digit || *i + 1 >= str.len || !is_digit(str.data[*i + 1])) {
                el_diag_report(
                    engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                    "digit separator '${sep}' must be between digits",
                    EL_DIAG_CHAR("sep", c)
                );
            }

            prev_digit = false;
            (*i)++;
        } else {
            break;
        }
    }
    return true;
}

static bool parse_frac_part(ElDiagEngine* engine, ElSourceSpan span, ElStringView str, usize* i, double* res, bool* has_digits) {
    if (*i >= str.len || str.data[*i] != '.') return true;
    (*i)++;
    double frac = 0.0; // NOLINT(readability-magic-numbers)
    double div = 1.0;  // NOLINT(readability-magic-numbers)
    bool prev_digit = false;
    while (*i < str.len) {
        char c = str.data[*i];
        if (is_digit(c)) {
            frac = (frac * 10.0) + (c - '0');  // NOLINT(readability-magic-numbers)
            div *= 10.0;                       // NOLINT(readability-magic-numbers)
            *has_digits = true;
            prev_digit = true;
            (*i)++;
        } else if (is_sep(c)) {
            if (!prev_digit || *i + 1 >= str.len || !is_digit(str.data[*i + 1])) {
                return el_diag_report(
                    engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                    "digit separator '${sep}' must be between digits",
                    EL_DIAG_CHAR("sep", c)
                );
            }

            prev_digit = false;
            (*i)++;
        } else {
            break;
        }
    }
    *res += frac / div;
    return true;
}

static bool parse_exp_part(ElDiagEngine* engine, ElSourceSpan span, ElStringView str, usize* i, double* res) {
    if (*i >= str.len || (str.data[*i] != 'e' && str.data[*i] != 'E')) return true;
    (*i)++;
    bool neg = false;
    if (*i < str.len && (str.data[*i] == '-' || str.data[*i] == '+')) {
        if (str.data[*i] == '-') neg = true;
        (*i)++;
    }
    if (*i >= str.len || !is_digit(str.data[*i])) {
        return el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
            "exponent must contain at least one digit"
        );
    }

    long long exp = 0;
    bool prev_digit = false;
    while (*i < str.len) {
        char c = str.data[*i];
        if (is_digit(c)) {
            exp = (exp * 10) + (c - '0'); // NOLINT(readability-magic-numbers)
            prev_digit = true;
            (*i)++;
        } else if (is_sep(c)) {
            if (!prev_digit || *i + 1 >= str.len || !is_digit(str.data[*i + 1])) {
                return el_diag_report(
                    engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
                    "digit separator '${sep}' must be between digits",
                    EL_DIAG_CHAR("sep", c)
                );
            }
            prev_digit = false;
            (*i)++;
        } else {
            break;
        }
    }

    double mult = 1.0;  // NOLINT(readability-magic-numbers)
    double base = 10.0; // NOLINT(readability-magic-numbers)
    long long e = exp;
    while (e > 0) {
        if (e % 2 == 1) mult *= base;
        base *= base;
        e /= 2;
    }
    if (neg) *res /= mult;
    else *res *= mult;
    return true;
}

double el_string_to_double(ElDiagEngine* engine, ElStringView str, ElSourceSpan span) {
    EL_ASSERT(str.len != 0, "empty string passed to el_string_to_u128");

    usize i = 0;
    bool neg = false;
    if (str.data[i] == '-') {
        neg = true;
        i++;
    } else if (str.data[i] == '+') {
        i++;
    }

    if (i >= str.len) {
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
            "numeric literal consists only of a sign; digits are required"
        );
        return 0;
    }

    double res = 0.0;
    bool has_digits = false;
    if (!parse_int_part(engine, span, str, &i, &res, &has_digits)) return 0;
    if (!parse_frac_part(engine, span, str, &i, &res, &has_digits)) return 0;
    if (!parse_exp_part(engine, span, str, &i, &res)) return 0;

    if (i != str.len || !has_digits) {
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-number", span,
            "numeric literal has an invalid format"
        );
        return 0;
    }
    return neg ? -res : res;
}
