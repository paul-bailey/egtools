#include "eg-devel.h"

uint32_t
eg_fletcher32(uint16_t *data, size_t len)
{
        uint32_t sum1 = 0xffff, sum2 = 0xffff;

        while (len) {
                /*
                 * 360 additions are the most that can be performed
                 * before having to take into account the modulo.
                 */
                unsigned tlen = len > 360 ? 360 : len;
                len -= tlen;
                do {
                        sum1 += *data++;
                        sum2 += sum1;
                } while (--tlen);

                /*
                 * The modulo 0xFFFF can be performed by adding the
                 * carryouts from a standard (modulo 0x10000) addition
                 * back into the lower halfword.
                 */
                sum1 = (sum1 & 0xffff) + (sum1 >> 16);
                sum2 = (sum2 & 0xffff) + (sum2 >> 16);
        }
        /* Second reduction step to reduce sums to 16 bits */
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);

        return (sum2 << 16) | sum1;
}
