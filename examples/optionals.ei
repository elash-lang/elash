int main() {
    int? opt1 = 123;
    int? opt2 = null;

    int value1 = opt1!; // unwrap operator: 123
    //int value2 = opt2!; // undefined behavior in release mode,
                          // panic in debug mode

    int thing = opt1 ?? opt2 ?? 47; // 123
    opt1 = null;
    int stuff = opt1 ?? opt2 ?? 47; // 47

    //int thingyThing = opt1 ?? opt2; // error
    int? thingyThing = opt1 ?? opt2;

    int n = 0;

    //int&? ref1 = null; // error, references are non nullable by default
    int&? ref1 = null;
    int&? ref2 = &n;

    //int value = ref2^; // error: cannot directly dereference optional type
    int value = ref2!^; // unwrap then dereference
}
