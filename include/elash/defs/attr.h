#pragma once

#if defined(__has_attribute)
#  define EL_HAS_ATTR(x) __has_attribute(x)
#else
#  define EL_HAS_ATTR(x) 0
#endif

#if defined(__has_c_attribute)
#  define EL_HAS_C_ATTR(x) __has_c_attribute(x)
#else
#  define EL_HAS_C_ATTR(x) 0
#endif

// --- nodiscard ---
#if EL_HAS_C_ATTR(nodiscard)
#  define EL_ATTR_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#  define EL_ATTR_NODISCARD _Check_return_
#else
#  define EL_ATTR_NODISCARD
#endif

// --- deprecated ---
#if EL_HAS_C_ATTR(deprecated)
#  define EL_ATTR_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#  define EL_ATTR_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#  define EL_ATTR_DEPRECATED(msg)
#endif

// --- maybe_unused ---
#if EL_HAS_C_ATTR(maybe_unused)
#  define EL_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#  define EL_UNUSED __attribute__((unused))
#else
#  define EL_UNUSED
#endif

// --- noreturn ---
#if EL_HAS_C_ATTR(noreturn)
#  define EL_ATTR_NORETURN [[noreturn]]
#elif defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#  define EL_ATTR_NORETURN __declspec(noreturn)
#else
#  define EL_ATTR_NORETURN
#endif

// --- printf format ---
#if defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_PRINTF_LIKE(fmt_idx, first_arg_idx) __attribute__((format(printf, fmt_idx, first_arg_idx)))
#else
#  define EL_ATTR_PRINTF_LIKE(fmt_idx, first_arg_idx)
#endif

// --- malloc ---
#if defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_MALLOC __attribute__((malloc))
#elif defined(_MSC_VER)
#  define EL_ATTR_MALLOC __declspec(restrict)
#else
#  define EL_ATTR_MALLOC
#endif

// --- inline ---
#if defined(_MSC_VER)
#  define EL_ATTR_ALWAYS_INLINE __forceinline
#  define EL_ATTR_INLINE __inline
#elif defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_ALWAYS_INLINE __attribute__((always_inline)) inline
#  define EL_ATTR_INLINE inline
#else
#  define EL_ATTR_ALWAYS_INLINE inline
#  define EL_ATTR_INLINE inline
#endif

// --- cold ---
#if defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_COLD __attribute__((cold))
#else
#  define EL_ATTR_COLD
#endif

// --- returns-nonnull ---
#if EL_HAS_ATTR(returns_nonnull)
#  define EL_ATTR_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#  define EL_ATTR_RETURNS_NONNULL
#endif

// --- nonnull ---
#if defined(__GNUC__) || defined(__clang__)
#  define EL_ATTR_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#  define EL_ATTR_NONNULL(...)
#endif

