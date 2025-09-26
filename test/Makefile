CC = gcc
CFLAGS = -g -Wall -Werror -I../src

SRCS = \
  fake_hal.c \
  ../src/bug.c bug_test.c


test: run_bug_test

run_bug_test: bug_test
	./bug_test

bug_test: bug.o bug_test.o fake_hal.o
	$(CC) bug_test.o bug.o fake_hal.o  -o bug_test -lm

include .test_depend

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > .test_depend

clean:
	rm -f *.o *_test *~
