CFLAGS=-std=c11 -g -static
SRC_FILES=$(wildcard *.c)
EXCLUDE_FILES=func.c
SRCS=$(filter-out $(EXCLUDE_FILES), $(SRC_FILES))
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
		./test.sh

func_test: 9cc
		./func_test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test func_test clean