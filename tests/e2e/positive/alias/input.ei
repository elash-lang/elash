// pascal reference???
alias TInteger = int;
alias PInteger = TInteger&;

TInteger add(int a, TInteger b) {
    return a + b;
}

void addInPlace(PInteger a, TInteger b) {
    a^ += b;
}

alias thing = add;

int result;
alias r = result;

alias IntArray = int[3];
int sum(IntArray arr) {
    return arr[0] + arr[1] + arr[2]; // very advanced
}

alias MyStructure = struct {
    alias SecretType = bool;
    SecretType foo;
};

int main() {
    if (&add != &thing) return 1;
    if (&result != &r)  return 2;

    result = add(1, 3);
    int x = thing(1, 3);

    if (r != x || result != x) {
        return 3;
    }

    result = 10;
    addInPlace(&result, 20);
    if (r != 30) {
        return 4;
    }

    int y = sum(IntArray { 15, 8, 1 });
    if (y != 24) {
        return 5;
    }

    MyStructure strct;
    strct.foo = true;
    if (!strct.foo) {
        return 6;
    }

    return 0;
}
