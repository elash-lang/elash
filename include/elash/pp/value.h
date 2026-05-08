#pragma once

#include <elash/defs/sv.h>

#include <elash/pp/valarr.h>

#include <elash/lexer/tokbuf.h>

typedef enum ElPpValueType {
    EL_PP_VAR_INT,
    EL_PP_VAR_FLOAT,
    EL_PP_VAR_BOOL,
    EL_PP_VAR_CHAR,
    EL_PP_VAR_STRING,
    EL_PP_VAR_ARRAY,
    EL_PP_VAR_TOKENS,
} ElPpValueType;

/// @brief Checks if a given ElPpVarType is a trivial type.
/// @param type The ElPpVarType to check.
/// @return True if the type is trivial (e.g., int, float, bool, char, string view), false otherwise.
bool el_pp_value_type_is_trivial(ElPpValueType type);

typedef struct ElPpValue {
    union {
        int as_int;          // EL_PP_VAR_INT
        float as_float;      // EL_PP_VAR_FLOAT
        bool as_bool;        // EL_PP_VAR_BOOL
        char as_char;        // EL_PP_VAR_CHAR
        ElStringView as_str; // EL_PP_VAR_STRING
        ElPpValueArr as_arr; // EL_PP_VAR_ARRAY
        ElTokenBuf as_toks;  // EL_PP_VAR_TOKENS
    };
    ElPpValueType type;
} ElPpValue;

/// @brief Frees any dynamically allocated resources held by an ElPpValue.
/// @param var A pointer to the ElPpValue whose resources are to be freed.
void el_pp_value_free(ElPpValue* val);
/// @brief Moves the content of a source ElPpValue to a destination ElPpValue.
/// @param src A pointer to the source ElPpValue, which will be invalidated for non-trivial types.
/// @param dst A pointer to the destination ElPpValue.
void el_pp_value_move(ElPpValue* src, ElPpValue* dst);
/// @brief Creates a deep copy of a source ElPpValue into a destination ElPpValue.
/// @param src A pointer to the source ElPpValue to copy from.
/// @param dst A pointer to the destination ElPpValue to copy to.
/// @return True if the copy is successful, false otherwise (e.g., memory allocation failure for array types).
bool el_pp_value_copy(const ElPpValue* src, ElPpValue* dst);

typedef struct ElPpVar {
    ElStringView name;
    ElPpValue value;
} ElPpVar;

/// @brief Frees any dynamically allocated resources held by an ElPpVar.
/// @param var A pointer to the ElPpVar whose resources are to be freed.
void el_pp_var_free(ElPpVar* var);
/// @brief Moves the content of a source ElPpVar to a destination ElPpVar.
/// @param src A pointer to the source ElPpVar, which will be invalidated for non-trivial types.
/// @param dst A pointer to the destination ElPpVar.
void el_pp_var_move(ElPpVar* src, ElPpVar* dst);
/// @brief Creates a deep copy of a source ElPpVar into a destination ElPpVar.
/// @param src A pointer to the source ElPpVar to copy from.
/// @param dst A pointer to the destination ElPpVar to copy to.
/// @return True if the copy is successful, false otherwise (e.g., memory allocation failure for array types).
bool el_pp_var_copy(const ElPpVar* src, ElPpVar* dst);


