bin_PROGRAMS = sizeof egfortune tree ts2date atab
egfortune_SOURCES = fortune.c
egfortune_CPPFLAGS = -DDATADIR=\"$(datadir)\"
sizeof_SOURCES = sizeof.c
tree_SOURCES = tree.c
if HAVE_LOFF_T
sizeof_CPPFLAGS = -DHAVE_LOFF_T=1
else
sizeof_CPPFLAGS = -DHAVE_LOFF_T=0
endif
EXTRA_DIST = fortunes
appdatadir = $(datadir)
appdata_DATA = fortunes
ts2date_SOURCES = ts2date.c
atab_SOURCES = atab.c
