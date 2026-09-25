#pragma once

typedef enum ElMutabilitySpec {
    EL_MUTSPEC_DEFAULT, ///< Readable and writeable
    EL_MUTSPEC_CONST,   ///< Read only
    EL_MUTSPEC_WONLY,   ///< Write only
} ElMutabilitySpec;
