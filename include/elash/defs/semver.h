#pragma once

#include <elash/defs/sv.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct ElSemVersion {
    uint16_t major, minor, patch;
} ElSemVersion;

#define EL_SEM_VER(MAJ, MIN, PAT) \
    ((ElSemVersion) { .major = (MAJ), .minor = (MIN), .patch = (PAT) })

static inline uint64_t el_semver_to_u64(ElSemVersion ver) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    return ((uint64_t) ver.major << 32) | ((uint64_t) ver.minor << 16) | (uint64_t) ver.patch;
}

static inline bool el_semver_at_least(ElSemVersion ver, ElSemVersion min) {
    return el_semver_to_u64(ver) >= el_semver_to_u64(min);
}

