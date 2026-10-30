#pragma once

// see https://sourceware.org/pipermail/libc-alpha/2025-November/172715.html
// similar trick but assumes CALL returns an already const pointer
#define EL_CONST_GENERIC(PTR, TYPE, CALL) \
    _Generic(0 ? (PTR) : (void *) 1,      \
       const void *: (CALL),              \
       void *: (TYPE*) CALL               \
    )
