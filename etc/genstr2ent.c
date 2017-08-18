#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Keep this table ordered, for fast string lookups */
static const struct entity_lut_t {
        int c;
        const char *s;
} ENTITY_TBL[] = {
        { '&',  "AMP" }, { '`',  "DiacriticalGrave" }, { '>',  "GT" }, { '<',  "LT" },
        { '\t', "Tab" }, { '|',  "VerticalLine" }, { '&',  "amp" }, { '\'', "apos" },
        { '*',  "ast" }, { ':',  "colon" }, { ',',  "comma" }, { '@',  "commat" },
        { '$',  "dollar" }, { '=',  "equals" }, { '`',  "grave" }, { '>',  "gt" },
        { '^',  "hat" }, { '{',  "lbrace" }, { '[',  "lbrack" }, { '{',  "lcub" },
        { '_',  "lowbar" }, { '(',  "lpar" }, { '[',  "lsqb" }, { '<',  "lt" },
        { '*',  "midast" }, { ' ',  "nbsp" }, { '#',  "num" }, { '%',  "percent" },
        { '.',  "period" }, { '+',  "plus" }, { '?',  "quest" }, { '}',  "rbrace" },
        { ']',  "rbrack" }, { '}',  "rcub" }, { ')',  "rpar" }, { ']',  "rsqb" },
        { ';',  "semi" }, { '/',  "sol" }, { '|',  "verbar" }, { '|',  "vert" },
        { 0, NULL },
};

static void
printchar(int c)
{
        if (isgraph(c)) {
                switch (c) {
                case '\'':
                        printf("'\\''");
                        break;
                case '\\':
                        printf("'\\\\'");
                        break;
                default:
                        printf("'%c'", c);
                        break;
                }
        } else {
                printf("%d", c);
        }
}

#define TAB "        "
int main(void)
{
        const char *cent[128];
        int indices[128];
        int i;

        for (i=0; i<128; i++) {
                cent[i] = NULL;
                indices[i] = -1;
        }

        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                int *pidx = &indices[(int)ENTITY_TBL[i].s[0]];
                cent[ENTITY_TBL[i].c] = ENTITY_TBL[i].s;
                if (*pidx < 0)
                        *pidx = i;
        }

        printf("static const struct entity_lut1_t {\n"
               TAB "int i;\n"
               TAB "const char *s;\n"
               "} ENTITY1_TBL_[] = {\n");
        for (i = 0; i < 128; i++) {
                printf(TAB "{ %d, ", indices[i]);
                if (cent[i] == NULL)
                        printf("NULL");
                else
                        printf("\"%s\"", cent[i]);
                printf("} /* ");
                printchar(i);
                printf(" */\n");
        }
        printf("};\n\n");

        printf("static const struct entity_lut_t {\n"
               TAB "int c;\n" TAB "const char *s;\n"
               "} ENTITY_TBL[] = {\n");
        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                printf(TAB "{ ");
                printchar(ENTITY_TBL[i].c);
                printf(", \"%s\" },\n", ENTITY_TBL[i].s);
        }
        printf("};\n\n");

        return 0;
}
