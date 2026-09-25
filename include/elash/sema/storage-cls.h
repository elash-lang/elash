#pragma once

typedef enum ElStorageClass {
    EL_STORAGECLS_LOCAL,  ///< Equivalent to 'auto' in C
    EL_STORAGECLS_STATIC, ///< Equivalent to 'static' in C
    // EL_STORAGECLS_TLS,
    // note that there is no internal/extern here.
    // in elash, storage classes and linkage specifiers
    // are separate concepts unlike in C
} ElStorageClass;
