
PROGS = json_test
CFLAGS += -DJSON_TEST__=1 -Wall

.PHONY: all clean

all: $(PROGS)

json.c: json.h

json_test: test_json.c json.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	$(if $(wildcard $(PROGS)),$(RM) $(PROGS))
