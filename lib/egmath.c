#include <stdio.h>
#include <math.h>
#include <egmath.h>

#ifndef abs
# define abs(x_) abs__(x_)
#endif

static inline double abs__(double x)
{
        if (x < 0.0)
                return -x;
        return x;
}

static void
build_fraction(struct eg_fraction_t *f, unsigned long *cf, int size)
{
        unsigned long num = 1;
        unsigned long den = 0;
        int i;
        for (i = size; i >= 0; i--) {
                unsigned long new_num = (unsigned long)cf[i] * num + den;
                den = num;
                num = new_num;
        }
        f->num = num;
        f->den = den;
        f->val = (double)num / (double)den;
}

/**
 * eg_fraction - Get quantized numerator and denominator for a floating
 *          point number.
 * @x: Value to rationalize
 * @f: Pointer to structure to store result.  The .val field of @f will
 *     contain the result of dividing the .num and .den fields of @f.
 *     This can be compared against @x to verify precision.
 */
void
eg_fraction(double x, struct eg_fraction_t *f)
{
        enum { CFSIZ = 40, };

        /*
         * array of truncated terms,
         * whose size starts at 1 and can grow to CFSIZ.
         */
        unsigned long cf[CFSIZ];

        double term, xabs, old_error;
        int i;

        old_error = xabs = term = abs(x);

        for (i = 0; i < CFSIZ; i++) {
                double new_error;

                cf[i] = (unsigned long)term;
                build_fraction(f, cf, i);

                /* check error */
                new_error = abs(f->val - xabs);
                if (i != 0 && cf[i] != 0 && new_error >= old_error) {
                        /*
                         * New error is bigger than old error.
                         * This means that the precision limit has been
                         * reached. Pop this (useless) term and break out.
                         *
                         * TODO: Is it faster to have two eg_fraction_t
                         * structs on stack, switch between them per
                         * iteration, and memcpy() if we get here?
                         *
                         * REVISIT: Why ">=" instead of ">" ?
                         */
                        build_fraction(f, cf, i - 1);
                        break;
                }

                old_error = new_error;
                if (new_error == 0.0) /* Unlikely, but... */
                        break;

                term -= (double)cf[i];
                term = 1.0 / term;
        }

        if (x < 0.0) {
                f->val = -f->val;
                f->num = -f->num;
        }
}
