// dummy test cases to see if test system works
#include <criterion/criterion.h>

Test(dummy, pass) {
    cr_assert(1 == 1, "1 should be equal to 1");
} 

Test(dummy, fail) {
    cr_assert(1 == 2, "1 should not be equal to 2");
}
