#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/platform.h>
#include <elash/defs/sv.h>

#include <stdbool.h>
#include <stdint.h>

#if EL_COMPILER_IS_MSVC
#include <intrin.h>
#endif

// Here's a cool trick that I learned on reddit.
// People yap about how `inline` in C is broken and how annoying it is that you need to write `static inline`. People are wrong. Not a big surprise.
//
// `inline` in C is very logical and actually does something useful. We can rely on this behavior to improve performance.
// the corresponding .c file defines `_EL_I128_API` as an empty macro, so including the header pulls ib the function definitions
// making them present in the object file with external linkage. By default when included by other files, this header defines `_EL_I128_API`
// as `inline`, which basically means if the compiler wants to inline it does, if it doesn't, it emits a regular out-of-line call referencing
// the symbols from object file that int128.c was compiled to.
//
// The funny thing is that C++ tries to fix it, which effectively removes this useful behavior from the language completly,
// there is no equivalent form to this in C++. C++ community knows nothing about C (and language design in general). Not a big surprise again!
//
// Let's call it "The Inline Trick" cause it sounds really cool. Don't judge me.
// Is this overengineering? Yes. Is this premature optimization? Yes.
#ifndef _EL_I128_API
    #define _EL_I128_API inline
#endif

// (the .c file defines EL_INT128_EMULATE so the emulated functions are always available, as native versions are really small,
// they are marked as static inline so they are [almost] always inlined)

// works on gcc, clang and (the best compiler, 100% not ragebait) icx
#if !defined(EL_INT128_EMULATE) && (defined(__SIZEOF_INT128__) || defined(__BITINT_MAXWIDTH__))
    #define EL_INT128_NATIVE 1

    // wrap it into a struct to block using operators like + directly, so we
    // don't accidentally write non portable code that will fail to compile
    // on shitty platforms like windows.
    #if defined(__SIZEOF_INT128__)
        typedef struct {
            signed __int128 v;
        } ElInt128;
        typedef struct {
            unsigned __int128 v;
        } ElUint128;
    #else
        // see https://cppreference.com/c/23
        typedef struct {
            signed _BitInt(128) v;
        } ElInt128;
        typedef struct {
            unsigned _BitInt(128) v;
        } ElUint128;
    #endif

    #define EL_INT128(V)      ((ElInt128) { .v = (V) })
    #define EL_INT128_H(L, H) ((ElInt128) { .v = (((signed __int128)(H)) << 64) | (L) })

    #define EL_UINT128(V)      ((ElUint128) { .v = (V) })
    #define EL_UINT128_H(L, H) ((ElUint128) { .v = (((unsigned __int128)(H)) << 64) | (L) })

    #define INT128_MIN         EL_INT128_H(0ULL, 0x8000000000000000ULL)
    #define INT128_MAX         EL_INT128_H(0xFFFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL)
    #define UINT128_MAX        EL_UINT128_H(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL)

    #if EL_GCC_EXTENSIONS
        #pragma push_macro("i128")
        #pragma push_macro("u128")
    #endif
    #define i128(V) ((ElInt128) { .v = (V) })
    #define u128(V) ((ElUint128) { .v = (V) })

    ///////////// Arithmetic Operators
    static inline ElInt128 el_i128_add(ElInt128 a, ElInt128 b) { return i128(a.v + b.v); }
    static inline ElInt128 el_i128_sub(ElInt128 a, ElInt128 b) { return i128(a.v - b.v); }
    static inline ElInt128 el_i128_mul(ElInt128 a, ElInt128 b) { return i128(a.v * b.v); }
    static inline ElInt128 el_i128_div(ElInt128 a, ElInt128 b) { return i128(a.v / b.v); }
    static inline ElInt128 el_i128_mod(ElInt128 a, ElInt128 b) { return i128(a.v % b.v); }
    static inline ElInt128 el_i128_neg(ElInt128 x)             { return i128(-x.v); }

    static inline ElUint128 el_u128_add(ElUint128 a, ElUint128 b) { return u128(a.v + b.v); }
    static inline ElUint128 el_u128_sub(ElUint128 a, ElUint128 b) { return u128(a.v - b.v); }
    static inline ElUint128 el_u128_mul(ElUint128 a, ElUint128 b) { return u128(a.v * b.v); }
    static inline ElUint128 el_u128_div(ElUint128 a, ElUint128 b) { return u128(a.v / b.v); }
    static inline ElUint128 el_u128_mod(ElUint128 a, ElUint128 b) { return u128(a.v % b.v); }
    static inline ElUint128 el_u128_neg(ElUint128 x)              { return u128(-x.v); }

    ///////////// Bitwise Operators
    static inline ElInt128 el_i128_and(ElInt128 a, ElInt128 b) { return i128(a.v & b.v);    }
    static inline ElInt128 el_i128_or(ElInt128 a, ElInt128 b)  { return i128(a.v | b.v);    }
    static inline ElInt128 el_i128_xor(ElInt128 a, ElInt128 b) { return i128(a.v ^ b.v);    }
    static inline ElInt128 el_i128_shl(ElInt128 a, int shift)  { return i128(a.v << shift); }
    static inline ElInt128 el_i128_shr(ElInt128 a, int shift)  { return i128(a.v >> shift); }
    static inline ElInt128 el_i128_not(ElInt128 x)             { return i128(~x.v);         }

    static inline ElUint128 el_u128_and(ElUint128 a, ElUint128 b) { return u128(a.v & b.v);    }
    static inline ElUint128 el_u128_or(ElUint128 a, ElUint128 b)  { return u128(a.v | b.v);    }
    static inline ElUint128 el_u128_xor(ElUint128 a, ElUint128 b) { return u128(a.v ^ b.v);    }
    static inline ElUint128 el_u128_shl(ElUint128 a, int shift)   { return u128(a.v << shift); }
    static inline ElUint128 el_u128_shr(ElUint128 a, int shift)   { return u128(a.v >> shift); }
    static inline ElUint128 el_u128_not(ElUint128 x)              { return u128(~x.v);         }

    //////////// Comparison Operators
    static inline bool el_i128_eq(ElInt128 a, ElInt128 b) { return a.v == b.v; }
    static inline bool el_i128_ne(ElInt128 a, ElInt128 b) { return a.v != b.v; }
    static inline bool el_i128_gt(ElInt128 a, ElInt128 b) { return a.v >  b.v; }
    static inline bool el_i128_lt(ElInt128 a, ElInt128 b) { return a.v <  b.v; }
    static inline bool el_i128_ge(ElInt128 a, ElInt128 b) { return a.v >= b.v; }
    static inline bool el_i128_le(ElInt128 a, ElInt128 b) { return a.v <= b.v; }

    static inline bool el_u128_eq(ElUint128 a, ElUint128 b) { return a.v == b.v; }
    static inline bool el_u128_ne(ElUint128 a, ElUint128 b) { return a.v != b.v; }
    static inline bool el_u128_gt(ElUint128 a, ElUint128 b) { return a.v >  b.v; }
    static inline bool el_u128_lt(ElUint128 a, ElUint128 b) { return a.v <  b.v; }
    static inline bool el_u128_ge(ElUint128 a, ElUint128 b) { return a.v >= b.v; }
    static inline bool el_u128_le(ElUint128 a, ElUint128 b) { return a.v <= b.v; }

    // NOLINTBEGIN(readability-magic-numbers): i hate you.
    static inline uint64_t el_i128_lo(ElInt128 v)  { return (uint64_t)v.v;         }
    static inline uint64_t el_i128_hi(ElInt128 v)  { return (uint64_t)(v.v >> 64); }
    static inline uint64_t el_u128_lo(ElUint128 v) { return (uint64_t)v.v;         }
    static inline uint64_t el_u128_hi(ElUint128 v) { return (uint64_t)(v.v >> 64); }
    // NOLINTEND(readability-magic-numbers)

    static inline double el_i128_to_double(ElInt128 v) { return (double)v.v; }

    #undef i128
    #undef u128
    #if EL_GCC_EXTENSIONS
        #pragma pop_macro("i128")
        #pragma pop_macro("u128")
    #endif
#else
    #define EL_INT128_NATIVE 0

    typedef struct {
        uint64_t lo;
        uint64_t hi;
    } ElInt128;
    typedef struct {
        uint64_t lo;
        uint64_t hi;
    } ElUint128;

    #define EL_INT128(V)       ((ElInt128) { .lo = (uint64_t)(V), .hi = ((int64_t)(V) < 0) ? (uint64_t)-1LL : 0ULL })
    #define EL_INT128_H(L, H)  ((ElInt128) { .lo = (L), .hi = (H) })

    #define EL_UINT128(V)      ((ElUint128) { .lo = (V), .hi = 0 })
    #define EL_UINT128_H(L, H) ((ElUint128) { .lo = (L), .hi = (H) })

    #define INT128_MIN         EL_INT128_H(0ULL, 0x8000000000000000ULL)
    #define INT128_MAX         EL_INT128_H(0xFFFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL)
    #define UINT128_MAX        EL_UINT128_H(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL)

ElUint128 el_i128_abs_u128(ElInt128 v);

static inline uint64_t el_i128_lo(ElInt128 v)  { return v.lo; }
static inline uint64_t el_i128_hi(ElInt128 v)  { return v.hi; }
static inline uint64_t el_u128_lo(ElUint128 v) { return v.lo; }
static inline uint64_t el_u128_hi(ElUint128 v) { return v.hi; }

_EL_I128_API ElUint128 el_u128_neg(ElUint128 x);
_EL_I128_API bool el_i128_lt(ElInt128 lhs, ElInt128 rhs);

// emulated 128-bit arithmetic
// stolen from https://github.com/aeldidi/ElInt128/blob/main/int128.h
// credits to the original author - @aeldidi

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(readability-math-missing-parentheses)

_EL_I128_API bool el_u128_lt(ElUint128 lhs, ElUint128 rhs)
{
	if (lhs.hi == rhs.hi) {
		return lhs.lo < rhs.lo;
	}

	return lhs.hi < rhs.hi;
}

// Returns lhs > rhs.
_EL_I128_API bool el_u128_gt(ElUint128 lhs, ElUint128 rhs)
{
	if (lhs.hi == rhs.hi) {
		return lhs.lo > rhs.lo;
	}

	return lhs.hi > rhs.hi;
}

// Returns lhs < rhs.
_EL_I128_API bool el_i128_lt(ElInt128 lhs, ElInt128 rhs)
{
	union {
		uint64_t u;
		int64_t i;
	} cvt;
	union {
		uint64_t u;
		int64_t i;
	} cvt2;

	cvt.u = lhs.hi;
	cvt2.u = rhs.hi;

	if (cvt.i == cvt2.i) {
		return lhs.lo < rhs.lo;
	}

	return cvt.i < cvt2.i;
}

// Returns lhs > rhs.
_EL_I128_API bool el_i128_gt(ElInt128 lhs, ElInt128 rhs)
{
	union {
		uint64_t u;
		int64_t i;
	} cvt;
	union {
		uint64_t u;
		int64_t i;
	} cvt2;

	cvt.u = lhs.hi;
	cvt2.u = rhs.hi;

	if (cvt.i == cvt2.i) {
		return lhs.lo > rhs.lo;
	}

	return cvt.i > cvt2.i;
}

// Returns lhs == rhs.
_EL_I128_API bool el_i128_eq(ElInt128 lhs, ElInt128 rhs)
{
	return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

// Returns lhs == rhs.
_EL_I128_API bool el_u128_eq(ElUint128 lhs, ElUint128 rhs)
{
	return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

// Returns lhs != rhs.
_EL_I128_API bool el_i128_ne(ElInt128 lhs, ElInt128 rhs)
{
	return !el_i128_eq(lhs, rhs);
}

// Returns lhs != rhs.
_EL_I128_API bool el_u128_ne(ElUint128 lhs, ElUint128 rhs)
{
	return !el_u128_eq(lhs, rhs);
}

// Returns lhs <= rhs.
_EL_I128_API bool el_u128_le(ElUint128 lhs, ElUint128 rhs)
{
	return el_u128_lt(lhs, rhs) || el_u128_eq(lhs, rhs);
}

// Returns lhs >= rhs.
_EL_I128_API bool el_u128_ge(ElUint128 lhs, ElUint128 rhs)
{
	return el_u128_gt(lhs, rhs) || el_u128_eq(lhs, rhs);
}

// Returns lhs >= rhs.
_EL_I128_API bool el_i128_ge(ElInt128 lhs, ElInt128 rhs)
{
	return el_i128_gt(lhs, rhs) || el_i128_eq(lhs, rhs);
}

// Returns lhs <= rhs.
_EL_I128_API bool el_i128_le(ElInt128 lhs, ElInt128 rhs)
{
	return el_i128_lt(lhs, rhs) || el_i128_eq(lhs, rhs);
}

#if !defined(EL_INT128_EMULATE) && EL_COMPILER_IS_MSVC && _M_X64
    // msvc-specific intrinsics
    static inline ElInt128 el_i128_add(ElInt128 a, ElInt128 b) {
        ElInt128 res;
        unsigned char carry = _addcarry_u64(0, a.lo, b.lo, &res.lo);
        _addcarry_u64(carry, a.hi, b.hi, &res.hi);
        return res;
    }

    static inline ElUint128 el_u128_add(ElUint128 a, ElUint128 b) {
        ElUint128 res;
        unsigned char carry = _addcarry_u64(0, a.lo, b.lo, &res.lo);
        _addcarry_u64(carry, a.hi, b.hi, &res.hi);
        return res;
    }

    static inline ElInt128 el_i128_sub(ElInt128 a, ElInt128 b) {
        ElInt128 res;
        unsigned char borrow = _subborrow_u64(0, a.lo, b.lo, &res.lo);
        _subborrow_u64(borrow, a.hi, b.hi, &res.hi);
        return res;
    }

    static inline ElUint128 el_u128_sub(ElUint128 a, ElUint128 b) {
        ElUint128 res;
        unsigned char borrow = _subborrow_u64(0, a.lo, b.lo, &res.lo);
        _subborrow_u64(borrow, a.hi, b.hi, &res.hi);
        return res;
    }

    static inline ElInt128 el_i128_mul(ElInt128 a, ElInt128 b) {
        uint64_t hi_val;
        uint64_t lo_val = _umul128(a.lo, b.lo, &hi_val);
        return (ElInt128) {
            .lo = lo_val,
            .hi = (uint64_t)(hi_val + (int64_t)a.hi * (int64_t)b.lo + (int64_t)a.lo * (int64_t)b.hi)
        };
    }

    static inline ElUint128 el_u128_mul(ElUint128 a, ElUint128 b) {
        uint64_t hi_val;
        uint64_t lo_val = _umul128(a.lo, b.lo, &hi_val);
        return (ElUint128) {
            .lo = lo_val,
            .hi = hi_val + a.hi * b.lo + a.lo * b.hi
        };
    }

    static inline ElInt128 el_i128_neg(ElInt128 x) {
        ElInt128 res;
        unsigned char borrow = _subborrow_u64(0, 0, x.lo, &res.lo);
        _subborrow_u64(borrow, 0, x.hi, &res.hi);
        return res;
    }

    static inline ElUint128 el_u128_neg(ElUint128 x) {
        ElUint128 res;
        unsigned char borrow = _subborrow_u64(0, 0, x.lo, &res.lo);
        _subborrow_u64(borrow, 0, x.hi, &res.hi);
        return res;
    }
#else

// Adds two int128s, wrapping on overflow.
_EL_I128_API ElInt128 el_i128_add(ElInt128 lhs, ElInt128 rhs)
{
	ElInt128 result = {
		.lo = lhs.lo + rhs.lo,
		.hi = lhs.hi + rhs.hi,
	};

	if (result.lo < lhs.lo) {
		result.hi += 1;
	}

	return result;
}

// Adds two uint128s, wrapping on overflow.
_EL_I128_API ElUint128 el_u128_add(ElUint128 lhs, ElUint128 rhs)
{
	ElInt128 a = { lhs.lo, lhs.hi };
	ElInt128 b = { rhs.lo, rhs.hi };
	ElInt128 tmp = el_i128_add(a, b);
	return (ElUint128){
		.lo = tmp.lo,
		.hi = tmp.hi,
	};
}

// Returns -x.
_EL_I128_API ElInt128 el_i128_neg(ElInt128 x)
{
	return (ElInt128){
		.lo = ~x.lo + 1,
		.hi = ~x.hi + (x.lo == 0),
	};
}

// Returns -x.
_EL_I128_API ElUint128 el_u128_neg(ElUint128 x)
{
	ElInt128 a = { x.lo, x.hi };
	ElInt128 tmp = el_i128_neg(a);
	return (ElUint128){ tmp.lo, tmp.hi };
}

// Subtracts two int128s, wrapping on underflow.
_EL_I128_API ElInt128 el_i128_sub(ElInt128 lhs, ElInt128 rhs)
{
	return el_i128_add(lhs, el_i128_neg(rhs));
}

// Subtracts two uint128s, wrapping on underflow.
_EL_I128_API ElUint128 el_u128_sub(ElUint128 lhs, ElUint128 rhs)
{
	ElUint128 result = { .lo = lhs.lo - rhs.lo, .hi = lhs.hi - rhs.hi };
	if (lhs.lo < rhs.lo) {
		result.hi -= 1;
	}

	return result;
}

// Multiply two unsigned 64 bit integers and get the 128 bit result.
_EL_I128_API ElUint128 el_u128_mul64(uint64_t lhs, uint64_t rhs)
{
	// For a more detailed explanation of this algorithm, see el_u128_mul.

	// Split the lo 64 bits of lhs and rhs into its hi and lo 32 bits.
	uint64_t left_lo32 = lhs & UINT32_MAX;
	uint64_t left_hi32 = lhs >> 32;
	uint64_t right_lo32 = rhs & UINT32_MAX;
	uint64_t right_hi32 = rhs >> 32;

	// Compute each component of the product as the sum of multiple 32 bit
	// products.
	uint64_t lo_lo = left_lo32 * right_lo32;
	uint64_t lo_hi = left_lo32 * right_hi32;
	uint64_t hi_lo = left_hi32 * right_lo32;
	uint64_t hi_hi = left_hi32 * right_hi32;

	uint64_t carry =
		((lo_lo >> 32) + (lo_hi & UINT32_MAX) + (hi_lo & UINT32_MAX)) >>
		32; // we want the hi bits of that sum

	// Assemble the final product from these components, adding the carry
	// to the hi 64 bits.
	return (ElUint128){
		.lo = lo_lo + (lo_hi << 32) + (hi_lo << 32),
		.hi = hi_hi + (lo_hi >> 32) + (hi_lo >> 32) + carry,
	};
}

// Multiply two signed 64 bit integers and get the 128 bit result.
_EL_I128_API ElInt128 el_i128_mul64(int64_t lhs, int64_t rhs)
{
	union {
		uint64_t u;
		int64_t i;
	} cvt;

	cvt.i = lhs;
	uint64_t a = cvt.u;

	cvt.i = rhs;
	uint64_t b = cvt.u;

	ElUint128 tmp = el_u128_mul64(a, b);
	ElInt128 result = {
		.lo = tmp.lo,
		.hi = tmp.hi,
	};

	if (lhs < 0) {
		result.hi -= rhs;
	}

	if (rhs < 0) {
		result.hi -= lhs;
	}

	return result;
}

// Returns lhs * rhs.
_EL_I128_API ElUint128 el_u128_mul(ElUint128 lhs, ElUint128 rhs)
{
	// We want to compute ab given two 128 bit integers a and b.
	// Let x = 2^64, a = a1x + a2, b = b1x + b2, where a1 and a2 are the
	// hi and lo bits of a respectively, and b1 and b2 are the hi and
	// lo bits of b2 respectively.

	// Then we compute the partial 64x64 products:
	ElUint128 i = el_u128_mul64(lhs.lo, rhs.lo); // a2b2
	ElUint128 j = el_u128_mul64(lhs.lo, rhs.hi); // a2b1
	ElUint128 k = el_u128_mul64(lhs.hi, rhs.lo); // a1b2
	// ElUint128 l = el_u128_mul64(lhs.hi, rhs.hi); // a1b1

	// l would be necessary if we were to perform the full 256 bit product,
	// but we're only interested in the 128 bit wrapped result.

	// and combine them into the result like so:
	//       i1 i2
	//    j1 j2 00
	//    k1 k2 00
	// l1 l2 00 00 +
	// -----------
	// ...........

	ElUint128 tmp = el_u128_add( //
		i, // i1 i2
		(ElUint128){ .hi = j.lo } // j2 00
	);

	// We simply return the lo bits of what would be a 256 bit result,
	// since we only care about the loer bits.
	ElUint128 result = el_u128_add( //
		tmp, //
		(ElUint128){ .hi = k.lo } // k2 00
	);

	// We would also compute the carry if multiplying into 256 bits, so we
	// could propagate it to the next additions.
	// uint64_t carry = (tmp.lo < lhs.lo) + (result.lo < lhs.lo);

	return result;
}

// Returns lhs * rhs.
_EL_I128_API ElInt128 el_i128_mul(ElInt128 lhs, ElInt128 rhs)
{
	if (el_i128_eq(lhs, EL_INT128(-1))) {
		return el_i128_neg(rhs);
	}

	if (el_i128_eq(rhs, EL_INT128(-1))) {
		return el_i128_neg(lhs);
	}

	bool result_negative = false;
	if (el_i128_lt(lhs, EL_INT128(0))) {
		result_negative = !result_negative;
		lhs = el_i128_neg(lhs);
	}

	if (el_i128_lt(rhs, EL_INT128(0))) {
		result_negative = !result_negative;
		rhs = el_i128_neg(rhs);
	}

	ElUint128 a = { .lo = lhs.lo, .hi = lhs.hi };
	ElUint128 b = { .lo = rhs.lo, .hi = rhs.hi };
	ElUint128 tmp = el_u128_mul(a, b);
	ElInt128 result = { .lo = tmp.lo, .hi = tmp.hi };
	if (result_negative) {
		result = el_i128_neg(result);
	}

	return result;
}

#endif

// Returns ~x.
_EL_I128_API ElInt128 el_i128_not(ElInt128 x)
{
	return (ElInt128){
		.lo = ~x.lo,
		.hi = ~x.hi,
	};
}

// Returns ~x.
_EL_I128_API ElUint128 el_u128_not(ElUint128 x)
{
	return (ElUint128){
		.lo = ~x.lo,
		.hi = ~x.hi,
	};
}

// Returns lhs | rhs.
_EL_I128_API ElInt128 el_i128_or(ElInt128 lhs, ElInt128 rhs)
{
	return (ElInt128){
		.lo = lhs.lo | rhs.lo,
		.hi = lhs.hi | rhs.hi,
	};
}

// Returns lhs & rhs.
_EL_I128_API ElInt128 el_i128_and(ElInt128 lhs, ElInt128 rhs)
{
	return (ElInt128){
		.lo = lhs.lo & rhs.lo,
		.hi = lhs.hi & rhs.hi,
	};
}

// Returns lhs | rhs.
_EL_I128_API ElUint128 el_u128_or(ElUint128 lhs, ElUint128 rhs)
{
	return (ElUint128){
		.lo = lhs.lo | rhs.lo,
		.hi = lhs.hi | rhs.hi,
	};
}

// Returns lhs & rhs.
_EL_I128_API ElUint128 el_u128_and(ElUint128 lhs, ElUint128 rhs)
{
	return (ElUint128){
		.lo = lhs.lo & rhs.lo,
		.hi = lhs.hi & rhs.hi,
	};
}

// Returns lhs ^ rhs.
_EL_I128_API ElInt128 el_i128_xor(ElInt128 lhs, ElInt128 rhs)
{
	return (ElInt128){
		.lo = lhs.lo ^ rhs.lo,
		.hi = lhs.hi ^ rhs.hi,
	};
}

// Returns lhs ^ rhs.
_EL_I128_API ElUint128 el_u128_xor(ElUint128 lhs, ElUint128 rhs)
{
	return (ElUint128){
		.lo = lhs.lo ^ rhs.lo,
		.hi = lhs.hi ^ rhs.hi,
	};
}

// Returns lhs << rhs.
_EL_I128_API ElInt128 el_i128_shl(ElInt128 lhs, int rhs)
{
	if (rhs == 0)   return lhs;
	if (rhs >= 128) return EL_INT128(0);

	if (rhs >= 64) {
		return (ElInt128){ .lo = 0, .hi = lhs.lo << (rhs - 64) };
	}

	return (ElInt128){
		.lo = lhs.lo << rhs,
		.hi = (lhs.hi << rhs) | (lhs.lo >> (64 - rhs)),
	};
}

// Returns lhs << rhs.
_EL_I128_API ElUint128 el_u128_shl(ElUint128 lhs, int rhs)
{
	if (rhs == 0)   return lhs;
	if (rhs >= 128) return EL_UINT128(0);

	ElInt128 a = { .lo = lhs.lo, .hi = lhs.hi };
	ElInt128 result = el_i128_shl(a, rhs);
	return (ElUint128){ .lo = result.lo, .hi = result.hi };
}

// Returns lhs >> rhs.
_EL_I128_API ElUint128 el_u128_shr(ElUint128 lhs, int rhs)
{
	if (rhs == 0) {
		return lhs;
	}

	if (rhs >= 128) {
		return EL_UINT128(0);
	}

	if (rhs >= 64) {
		return (ElUint128){ .lo = lhs.hi >> (rhs - 64), .hi = 0 };
	}

	return (ElUint128){
		.lo = (lhs.lo >> rhs) | (lhs.hi << (64 - rhs)),
		.hi = (lhs.hi >> rhs),
	};
}

// Returns lhs >> rhs.
// On signed ElInt128 this is an arithmetic shift, so the sign bit is used for
// shifted in bits.
_EL_I128_API ElInt128 el_i128_shr(ElInt128 lhs, int rhs)
{
	if (rhs == 0) {
		return lhs;
	}

	if (rhs >= 128) {
		return el_i128_lt(lhs, EL_INT128(0)) ? EL_INT128(-1) : EL_INT128(0);
	}

	if (el_i128_ge(lhs, EL_INT128(0))) {
		ElUint128 a = { .lo = lhs.lo, .hi = lhs.hi };
		ElUint128 result = el_u128_shr(a, rhs);
		return (ElInt128){ .lo = result.lo, .hi = result.hi };
	}

	if (rhs >= 64) {
		int shift = rhs - 64;
		uint64_t lo = (uint64_t)((int64_t)lhs.hi >> shift);
		return (ElInt128){
			.lo = lo,
			.hi = UINT64_C(~0),
		};
	} else if (rhs == 64) {
		return (ElInt128){
			.lo = lhs.hi,
			.hi = UINT64_C(~0),
		};
	}

	return (ElInt128){
		.lo = (lhs.lo >> rhs) | (lhs.hi << (64 - rhs)),
		.hi = (lhs.hi >> rhs) | (UINT64_C(~0) << (64 - rhs)),
	};
}

// Returns lhs / rhs.
_EL_I128_API ElUint128 el_u128_div(ElUint128 lhs, ElUint128 rhs)
{
	// Based on the algorithm described here:
	// https://stackoverflow.com/questions/5386377/division-without-using
	if (el_u128_eq(rhs, EL_UINT128(0))) {
		return EL_UINT128(0);
	}

	if (el_u128_gt(rhs, lhs)) {
		return EL_UINT128(0);
	}

	if (el_u128_eq(lhs, rhs)) {
		return EL_UINT128(1);
	}

	ElUint128 current = EL_UINT128(1);
	ElUint128 result = EL_UINT128(0);
	ElUint128 divisor = rhs;
	bool overflowed = false;
	while (el_u128_le(divisor, lhs)) {
		ElUint128 next_divisor = el_u128_shl(divisor, 1);
		ElUint128 next_current = el_u128_shl(current, 1);
		if (el_u128_eq(next_divisor, EL_UINT128(0)) ||
		    el_u128_eq(next_current, EL_UINT128(0))) {
			overflowed = true;
			break;
		}

		divisor = next_divisor;
		current = next_current;
	}

	if (!overflowed) {
		divisor = el_u128_shr(divisor, 1);
		current = el_u128_shr(current, 1);
	}

	while (!el_u128_eq(current, EL_UINT128(0))) {
		if (el_u128_ge(lhs, divisor)) {
			lhs = el_u128_sub(lhs, divisor);
			result = el_u128_or(result, current);
		}

		current = el_u128_shr(current, 1);
		divisor = el_u128_shr(divisor, 1);
	}

	return result;
}

// Returns lhs % rhs.
_EL_I128_API ElUint128 el_u128_mod(ElUint128 lhs, ElUint128 rhs)
{
	// Based on the algorithm described here:
	// https://stackoverflow.com/questions/5386377/division-without-using
	if (el_u128_eq(rhs, EL_UINT128(0))) {
		return EL_UINT128(0);
	}

	if (el_u128_gt(rhs, lhs)) {
		return lhs;
	}

	if (el_u128_eq(lhs, rhs)) {
		return EL_UINT128(0);
	}

	ElUint128 current = EL_UINT128(1);
	ElUint128 result = EL_UINT128(0);
	ElUint128 divisor = rhs;
	bool overflowed = false;
	while (el_u128_le(divisor, lhs)) {
		ElUint128 next_divisor = el_u128_shl(divisor, 1);
		ElUint128 next_current = el_u128_shl(current, 1);
		if (el_u128_eq(next_divisor, EL_UINT128(0)) ||
		    el_u128_eq(next_current, EL_UINT128(0))) {
			overflowed = true;
			break;
		}

		divisor = next_divisor;
		current = next_current;
	}

	if (!overflowed) {
		divisor = el_u128_shr(divisor, 1);
		current = el_u128_shr(current, 1);
	}

	while (!el_u128_eq(current, EL_UINT128(0))) {
		if (el_u128_ge(lhs, divisor)) {
			lhs = el_u128_sub(lhs, divisor);
			result = el_u128_or(result, current);
		}

		current = el_u128_shr(current, 1);
		divisor = el_u128_shr(divisor, 1);
	}

	return lhs;
}

// Returns lhs / rhs.
_EL_I128_API ElInt128 el_i128_div(ElInt128 lhs, ElInt128 rhs)
{
	if (el_i128_eq(rhs, EL_INT128(0))) {
		return EL_INT128(0);
	}

	bool result_negative = el_i128_lt(lhs, EL_INT128(0)) != el_i128_lt(rhs, EL_INT128(0));
	ElUint128 a = el_i128_abs_u128(lhs);
	ElUint128 b = el_i128_abs_u128(rhs);
	ElUint128 c = el_u128_div(a, b);
	ElInt128 result = { .lo = c.lo, .hi = c.hi };

	if (result_negative) {
		return el_i128_neg(result);
	}

	return result;
}

// Returns lhs % rhs.
_EL_I128_API ElInt128 el_i128_mod(ElInt128 lhs, ElInt128 rhs)
{
	if (el_i128_eq(rhs, EL_INT128(0))) {
		return EL_INT128(0);
	}

	bool result_negative = el_i128_lt(lhs, EL_INT128(0));
	ElUint128 a = el_i128_abs_u128(lhs);
	ElUint128 b = el_i128_abs_u128(rhs);
	ElUint128 c = el_u128_mod(a, b);
	ElInt128 result = { .lo = c.lo, .hi = c.hi };

	if (result_negative && el_u128_ne(c, EL_UINT128(0))) {
		return el_i128_neg(result);
	}

	return result;
}


_EL_I128_API double el_i128_to_double(ElInt128 v) {
    const double TWO_POW_64 = 18446744073709551616.0;
    if ((int64_t)v.hi < 0) {
        ElUint128 u = el_i128_abs_u128(v);
        double d = (double)u.hi * TWO_POW_64 + (double)u.lo;
        return -d;
    } else {
        return (double)v.hi * TWO_POW_64 + (double)v.lo;
    }
}

// NOLINTEND(readability-math-missing-parentheses)
// NOLINTEND(readability-magic-numbers)

#endif

static inline ElUint128 el_i128_bitcast_u128(ElInt128 v)  { return EL_UINT128_H(el_i128_lo(v), el_i128_hi(v)); }
static inline ElInt128  el_u128_bitcast_i128(ElUint128 v) { return EL_INT128_H(el_u128_lo(v),  el_u128_hi(v)); }

#if EL_INT128_NATIVE
    // it is already declared in the emulated branch which causes linter warnings
    // warning: redundant 'el_i128_abs_u128' declaration [readability-redundant-declaration]
    ElUint128 el_i128_abs_u128(ElInt128 v);
#endif

// NOLINTBEGIN(readability-magic-numbers): clueless
_Static_assert(sizeof(ElInt128)  == 16, "ElInt128 should be exactly 16 bytes");
_Static_assert(sizeof(ElUint128) == 16, "ElUint128 should be exactly 16 bytes");
// NOLINTEND(readability-magic-numbers)

// NOLINTBEGIN(readability-magic-numbers)
ElStringView el_i128_to_string(ElInt128 x,  uint base, char* buf);
ElStringView el_u128_to_string(ElUint128 x, uint base, char* buf);
// NOLINTEND(readability-magic-numbers)

