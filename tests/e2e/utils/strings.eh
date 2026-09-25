bool streql(const char[] a, const char[] b) {
    if (len(a) != len(b)) return false;

    usize i = 0;
    while (i < len(a)) {
        if (a[i] != b[i]) return false;
        i += 1;
    }

    return true;
}
