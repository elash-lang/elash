int abs(int n) {
    if (n < 0) return -n;
    return n;
}

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

int branchlessMin(int a, int b) {
    return (a + b - abs(a - b)) / 2;
}

int main() {
    int a = 123;
    int b = 47;
    return branchlessMin(a, b);
}
