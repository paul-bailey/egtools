noinst_PROGRAMS = test_parse_date test_html test_eghash test_frac

localdir_ldadd = $(top_srcdir)/lib/libeg-devel.la
localdir_cppflags = -Wall -I$(top_srcdir)/include

test_html_SOURCES = test_html.c
test_html_CPPFLAGS = $(localdir_cppflags)
test_html_LDADD = $(localdir_ldadd)

test_eghash_SOURCES = test_eghash.c
test_eghash_CPPFLAGS = $(localdir_cppflags)
test_eghash_LDADD = $(localdir_ldadd)

test_parse_date_SOURCES = test_parse_date.c
test_parse_date_CPPFLAGS = $(localdir_cppflags)
test_parse_date_LDADD = $(localdir_ldadd)

test_frac_SOURCES = test_frac.c
test_frac_CPPFLAGS = $(localdir_cppflags)
test_frac_LDADD = $(localdir_ldadd)
