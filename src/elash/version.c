#include <elash/version.h>

ElSemVersion el_library_version() {
    return EL_VERSION_SEMVER;
}

ElStringView el_library_version_string() {
    return EL_SV(EL_VERSION_STRING);
}
