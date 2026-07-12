bool flag;

bool setTheFlag() {
    flag = true;
    return false; // dummy
}

int main() {
    flag = false;
    bool test;

    bool t = true;
    bool f = false;

    test = t || setTheFlag();
    if (flag) return 1;

    test = f && setTheFlag();
    if (flag) return 2;

    test = f => setTheFlag();
    if (flag) return 3;

    test = true;
    test ||= setTheFlag();
    if (flag) return 4;

    test = false;
    test &&= setTheFlag();
    if (flag) return 5;

    test = false;
    test =>= setTheFlag();
    if (flag) return 6;

    return 0;
}
