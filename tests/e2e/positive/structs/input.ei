int getX(struct(int, int)& p) {
    return p^.0;
}

int main() {
    struct { int x; int y; } point1;
    struct(int, int) point2;

    point1.x = 123;
    point1.y = 321;

    point2.0 = 123;
    point2.1 = 321;

    struct { int x; int y; } point3 = point1;

    if (point1.x != 123 || point1.y != point3.y) {
        return 1;
    }

    if (getX(&point2) != point2.0 || point2.1 != point3.y) {
        return 2;
    }

    return 0;
}
