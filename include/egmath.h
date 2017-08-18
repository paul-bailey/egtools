#ifndef EGMATH_H
#define EGMATH_H

#ifdef __cplusplus
extern "C" {
#endif

struct eg_fraction_t {
        unsigned long num;
        unsigned long den;
        double val;
};

extern void eg_fraction(double x, struct eg_fraction_t *f);

#ifdef __cplusplus
}
#endif

#endif /* EGMATH_H */
