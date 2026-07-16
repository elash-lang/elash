int sumElements(int[] slice) {
    int acc = 0;
    usize i = 0;
    while (i < len(slice)) {
        acc += slice[i];
        i += 1;
    }
    return acc;
}

int sumElementsRaw(int[&] raw, usize size) {
    return sumElements(mkslice(raw, size));
}

int main() {
    int[5] arr = { 10, 20, 30, 40, 50 };
    int sum = sumElements(arr);
    return sum - 150;
}
