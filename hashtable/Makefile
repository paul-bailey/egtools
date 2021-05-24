.PHONY: all clean

CFLAGS += -Wall -DTEST_HASHTABLE__
PROGS := test_hashtable

all: $(PROGS)

hashtable.c test_hashtable.c: hashtable.h

test_hashtable: hashtable.c test_hashtable.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) -rf $(PROGS)
