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
build_fraction(struct eg_fraction_t *f, unsigned long *cf, size_t size)
{
        unsigned long term = size;
        unsigned long num = cf[--term];
        unsigned long den = 1;

        while (term-- > 0) {
                unsigned long tmp = cf[term];
                unsigned long new_num = tmp * num + den;
                unsigned long new_den = num;

                num = new_num;
                den = new_den;
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
        double t = abs(x);
        double xa = t;
        double old_error = t;
        unsigned long cf[40];
        size_t size = 0;

        do {
                double new_error;
                unsigned long tmp;

                tmp = (unsigned long)t;
                cf[size++] = tmp;
                build_fraction(f, cf, size);

                /* check error */
                new_error = abs(f->val - xa);
                if (tmp != 0 && new_error >= old_error) {
                        /*
                         * New error is bigger than old error.
                         * This means that the precision limit has been
                         * reached. Pop this (useless) term and break out.
                         */
                        --size;
                        build_fraction(f, cf, size);
                        break;
                }
                old_error = new_error;
                if (new_error == 0)
                        break;
                t -= (double)tmp;
                t = 1 / t;
        } while (size < 39);

        if (x < 0.0) {
                f->val = -f->val;
                f->num = -f->num;
        }
}
