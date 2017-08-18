#include <egmath.h>
#include <stdio.h>

static double
myabs(double x)
{
        if (x < 0.0)
                return -x;
        return x;
}

static void
test_fraction(double x)
{
        struct eg_fraction_t f;
        eg_fraction(x, &f);
        printf("The fraction is:  %ld / %ld\n", f.num, f.den);
        printf("Target x = %0.16f\n", x);
        printf("Result   = %0.16f\n", f.val);
        printf("Relative error is: %e\n", myabs(f.val - x) / x);
        putchar('\n');
}

int
main(void)
{
    test_fraction(15.38 / 12.3);
    test_fraction(-15.38 / 12.3);
    test_fraction(0.3333333333333333333);    //  1 / 3
    test_fraction(0.4184397163120567376);    //  59 / 141
    test_fraction(0.8323518818409020299);    //  1513686 / 1818565
    test_fraction(3.1415926535897932385);    //  pi
}
