int main() {
    // basics
    float16 h = 1.5;
    float f = 2.5;
    float64 d = 3.5;

    if (h != 1.5) return 1;
    if (f != 2.5) return 2;
    if (d != 3.5) return 3;

    h = 5;
    f = 6;
    d = 7;

    if (h != 5.0) return 4;
    if (f != 6.0) return 5;
    if (d != 7.0) return 6;

    // math
    if ((1.5 + 2.5) != 4.0) return 10;
    if ((f + 2.0) != 8.0) return 11;
    if ((d + h as float64) != 12.0) return 12;

    if ((8.5 - 3.5) != 5.0) return 20;
    if ((f - 1.0) != 5.0) return 21;
    if ((d - h as float64) != 2.0) return 22;

    if ((2.0 * 4.0) != 8.0) return 30;
    if ((1.5 * 2.0) != 3.0) return 31;
    if ((f * 2.0) != 12.0) return 32;

    if ((8.0 / 2.0) != 4.0) return 40;
    if ((7.5 / 2.5) != 3.0) return 41;
    if ((d / 7.0) != 1.0) return 42;

    if ((+5.0) != 5.0) return 50;
    if ((-5.0) != -5.0) return 51;
    if (-(-5.0) != 5.0) return 52;

    // compound assignments
    f = 1.0;

    f += 2.0;
    if (f != 3.0) return 60;
    f -= 1.0;
    if (f != 2.0) return 61;
    f *= 5.0;
    if (f != 10.0) return 62;
    f /= 2.0;
    if (f != 5.0) return 63;

    // inc/dec
    f = 1.0;

    ++f;
    if (f != 2.0) return 70;
    f++;
    if (f != 3.0) return 71;
    --f;
    if (f != 2.0) return 72;
    f--;
    if (f != 1.0) return 73;

    if (f++ != 1.0) return 74;
    if (++f != 3.0) return 75;

    // comparisons
    if (!(2.0 == 2.0)) return 80;
    if (!(2.0 != 3.0)) return 81;
    if (!(2.0 < 3.0))  return 82;
    if (!(3.0 > 2.0))  return 83;
    if (!(2.0 <= 2.0)) return 84;
    if (!(2.0 >= 2.0)) return 85;

    // integer conversion
    int i;

    i = 3.75 as int;
    if (i != 3) return 90;

    f = 7 as int as float;
    if (f != 7.0) return 91;

    d = i as float64;
    if (d != 3.0) return 92;

    // negative zero
    f = -0.0;
    if (f != 0.0)
        return 140;

    // convertions
    h = 1.25;
    f = h as float;
    d = h as float64;

    if (f != 1.25) return 180;
    if (d != 1.25) return 181;

    f = 5.5;
    d = f as float64;
    if (d != 5.5)
        return 182;

    d = 9.25;
    f = d as float;
    if (f != 9.25)
        return 183;

    // final test lmao
    return 0 as float16 as float128 as float as int;
}
