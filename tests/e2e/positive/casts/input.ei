int main() {
    int n = 10;
    int32 d = n as int32;
    uint16 x = d as uint16;
    int64 y = x as int64;
    uint32 z = x; // implicit cast

    if (n != 10 || d != 10 || x != 10 || y != 10 || z != 10) {
        return 1;
    }

    char c = 'X';
    uint8 b = c as uint8;
    int32 h = c as int32;
    if (b != 88 as uint8 || h != 88 as int32) {
        return 2;
    }

    char c2 = b as char;
    if (c2 != 'X') {
        return 3;
    }

    int[3] arr = { 4, 5, 6 };
    int[&] stuff = arr;
    if (stuff[0] != 4 || stuff[1] != 5 || stuff[2] != 6) {
        return 4;
    }

    int[] slice = arr;
    if (len(slice) != len(arr) || slice[1] != 5) {
        return 5;
    }

    return 0;
}
