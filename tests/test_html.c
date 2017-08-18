#include <egxml.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static int
table_in_col(FILE *fp, int state, void *priv)
{
        printf("%s (state=%d)", (const char *)priv, state);
        return 0;
}

static int
table_one_col(FILE *fp, const char *colid, int state)
{
        int res;
        char colinfo[100];
        struct xml_runner_t runner = {
                .tag = xml_tag_create("td"),
                .priv = colinfo,
                .flags = 0,
                .cb = table_in_col,
        };
        assert(runner.tag != NULL);

        sprintf(colinfo, "Column number %c", colid[3]);
        res = xml_add_attribute(runner.tag, "id", colid);
        assert(res == 0);
        xml_tag_recursive(fp, state, &runner);
        xml_tag_free(runner.tag);
        assert(errno == 0);
        return 0;
}

static int
table_cols(FILE *fp, int state, void *priv)
{
        const char **colids = priv;
        while (*colids != NULL) {
                table_one_col(fp, *colids, state);
                ++colids;
        }
        return 0;
}

static int
table_one_row(FILE *fp, const char *rowid, int state)
{
        int res;
        const char *colids[] = {
                "col1", "col2", "col3", NULL,
        };
        struct xml_runner_t runner = {
                .tag = xml_tag_create("tr"),
                .priv = (void *)colids,
                .flags = XML_NL,
                .cb = table_cols,
        };

        assert(runner.tag != NULL);
        res = xml_add_attribute(runner.tag, "id", rowid);
        assert(res == 0);
        xml_tag_recursive(fp, state, &runner);
        xml_tag_free(runner.tag);
        assert(errno == 0);
        return 0;
}

static int
table_rows(FILE *fp, int state, void *priv)
{
        const char **rowids = priv;
        while (*rowids != NULL) {
                table_one_row(fp, *rowids, state);
                ++rowids;
        }
        return 0;
}

int main(void)
{
        int res;
        const char *rowids[] = {
                "row1", "row2", "row3", NULL,
        };
        struct xml_runner_t runner = {
                .tag = xml_tag_create("table"),
                .priv = rowids,
                .flags = XML_NL,
                .cb = table_rows,
        };
        assert(runner.tag != NULL);
        res = xml_add_attribute(runner.tag, "id", "mytable");
        assert(res == 0);
        xml_tag_recursive(stdout, 0, &runner);
        xml_tag_free(runner.tag);
        assert(errno == 0);
        return 0;
}
