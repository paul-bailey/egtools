/*
 * In the case of these entity lookups, a table-offset
 * followed by linear search is much faster than even a
 * binary search in almost every case.
 */
#include <egxml.h>
#include <string.h>

static const struct entity_lut1_t {
        /* First index of ENTITY2_TBL with matching s[0] */
        int i;
        /* String corresponding to char == this table's index */
        const char *s;
} ENTITY1_TBL[] = {
        { -1, NULL}, /* 0 */
        { -1, NULL}, /* 1 */
        { -1, NULL}, /* 2 */
        { -1, NULL}, /* 3 */
        { -1, NULL}, /* 4 */
        { -1, NULL}, /* 5 */
        { -1, NULL}, /* 6 */
        { -1, NULL}, /* 7 */
        { -1, NULL}, /* 8 */
        { -1, "Tab"}, /* '\t' */
        { -1, NULL}, /* 10 */
        { -1, NULL}, /* 11 */
        { -1, NULL}, /* 12 */
        { -1, NULL}, /* 13 */
        { -1, NULL}, /* 14 */
        { -1, NULL}, /* 15 */
        { -1, NULL}, /* 16 */
        { -1, NULL}, /* 17 */
        { -1, NULL}, /* 18 */
        { -1, NULL}, /* 19 */
        { -1, NULL}, /* 20 */
        { -1, NULL}, /* 21 */
        { -1, NULL}, /* 22 */
        { -1, NULL}, /* 23 */
        { -1, NULL}, /* 24 */
        { -1, NULL}, /* 25 */
        { -1, NULL}, /* 26 */
        { -1, NULL}, /* 27 */
        { -1, NULL}, /* 28 */
        { -1, NULL}, /* 29 */
        { -1, NULL}, /* 30 */
        { -1, NULL}, /* 31 */
        { -1, "nbsp"}, /* ' ' */
        { -1, NULL}, /* '!' */
        { -1, NULL}, /* '"' */
        { -1, "num"}, /* '#' */
        { -1, "dollar"}, /* '$' */
        { -1, "percent"}, /* '%' */
        { -1, "amp"}, /* '&' */
        { -1, "apos"}, /* '\'' */
        { -1, "lpar"}, /* '(' */
        { -1, "rpar"}, /* ')' */
        { -1, "midast"}, /* '*' */
        { -1, "plus"}, /* '+' */
        { -1, "comma"}, /* ',' */
        { -1, NULL}, /* '-' */
        { -1, "period"}, /* '.' */
        { -1, "sol"}, /* '/' */
        { -1, NULL}, /* '0' */
        { -1, NULL}, /* '1' */
        { -1, NULL}, /* '2' */
        { -1, NULL}, /* '3' */
        { -1, NULL}, /* '4' */
        { -1, NULL}, /* '5' */
        { -1, NULL}, /* '6' */
        { -1, NULL}, /* '7' */
        { -1, NULL}, /* '8' */
        { -1, NULL}, /* '9' */
        { -1, "colon"}, /* ':' */
        { -1, "semi"}, /* ';' */
        { -1, "lt"}, /* '<' */
        { -1, "equals"}, /* '=' */
        { -1, "gt"}, /* '>' */
        { -1, "quest"}, /* '?' */
        { -1, "commat"}, /* '@' */
        { 0, NULL}, /* 'A' */
        { -1, NULL}, /* 'B' */
        { -1, NULL}, /* 'C' */
        { 1, NULL}, /* 'D' */
        { -1, NULL}, /* 'E' */
        { -1, NULL}, /* 'F' */
        { 2, NULL}, /* 'G' */
        { -1, NULL}, /* 'H' */
        { -1, NULL}, /* 'I' */
        { -1, NULL}, /* 'J' */
        { -1, NULL}, /* 'K' */
        { 3, NULL}, /* 'L' */
        { -1, NULL}, /* 'M' */
        { -1, NULL}, /* 'N' */
        { -1, NULL}, /* 'O' */
        { -1, NULL}, /* 'P' */
        { -1, NULL}, /* 'Q' */
        { -1, NULL}, /* 'R' */
        { -1, NULL}, /* 'S' */
        { 4, NULL}, /* 'T' */
        { -1, NULL}, /* 'U' */
        { 5, NULL}, /* 'V' */
        { -1, NULL}, /* 'W' */
        { -1, NULL}, /* 'X' */
        { -1, NULL}, /* 'Y' */
        { -1, NULL}, /* 'Z' */
        { -1, "lsqb"}, /* '[' */
        { -1, NULL}, /* '\\' */
        { -1, "rsqb"}, /* ']' */
        { -1, "hat"}, /* '^' */
        { -1, "lowbar"}, /* '_' */
        { -1, "grave"}, /* '`' */
        { 6, NULL}, /* 'a' */
        { -1, NULL}, /* 'b' */
        { 9, NULL}, /* 'c' */
        { 12, NULL}, /* 'd' */
        { 13, NULL}, /* 'e' */
        { -1, NULL}, /* 'f' */
        { 14, NULL}, /* 'g' */
        { 16, NULL}, /* 'h' */
        { -1, NULL}, /* 'i' */
        { -1, NULL}, /* 'j' */
        { -1, NULL}, /* 'k' */
        { 17, NULL}, /* 'l' */
        { 24, NULL}, /* 'm' */
        { 25, NULL}, /* 'n' */
        { -1, NULL}, /* 'o' */
        { 27, NULL}, /* 'p' */
        { 30, NULL}, /* 'q' */
        { 31, NULL}, /* 'r' */
        { 36, NULL}, /* 's' */
        { -1, NULL}, /* 't' */
        { -1, NULL}, /* 'u' */
        { 38, NULL}, /* 'v' */
        { -1, NULL}, /* 'w' */
        { -1, NULL}, /* 'x' */
        { -1, NULL}, /* 'y' */
        { -1, NULL}, /* 'z' */
        { -1, "lcub"}, /* '{' */
        { -1, "vert"}, /* '|' */
        { -1, "rcub"}, /* '}' */
        { -1, NULL}, /* '~' */
        { -1, NULL} /* 127 */
};

static const struct entity_lut_t {
        int c;
        const char *s;
} ENTITY2_TBL[] = {
        { '&', "AMP" },
        { '`', "DiacriticalGrave" },
        { '>', "GT" },
        { '<', "LT" },
        { '\t', "Tab" },
        { '|', "VerticalLine" },
        { '&', "amp" },
        { '\'', "apos" },
        { '*', "ast" },
        { ':', "colon" },
        { ',', "comma" },
        { '@', "commat" },
        { '$', "dollar" },
        { '=', "equals" },
        { '`', "grave" },
        { '>', "gt" },
        { '^', "hat" },
        { '{', "lbrace" },
        { '[', "lbrack" },
        { '{', "lcub" },
        { '_', "lowbar" },
        { '(', "lpar" },
        { '[', "lsqb" },
        { '<', "lt" },
        { '*', "midast" },
        { ' ', "nbsp" },
        { '#', "num" },
        { '%', "percent" },
        { '.', "period" },
        { '+', "plus" },
        { '?', "quest" },
        { '}', "rbrace" },
        { ']', "rbrack" },
        { '}', "rcub" },
        { ')', "rpar" },
        { ']', "rsqb" },
        { ';', "semi" },
        { '/', "sol" },
        { '|', "verbar" },
        { '|', "vert" },
};

int
xml_str2ent(const char *s)
{
        int idx, res;
        const struct entity_lut_t *t;
        if ((unsigned)s[0] > 127)
                return EOF;

        idx = ENTITY1_TBL[(int)s[0]].i;
        if (idx < 0)
                return EOF;
        t = &ENTITY2_TBL[idx];
        while ((res = strcmp(t->s, s)) < 0)
                ++t;
        if (res != 0)
                return EOF;
        return t->c;
}

const char *
xml_ent2str(int c)
{
        if ((unsigned)c > 127)
                return NULL;
        return ENTITY1_TBL[c].s;
}
