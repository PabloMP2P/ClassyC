
CC = gcc
CFLAGS = -I../ -Iunity
UNITY_SRC = unity.c
SRC = ../ClassyC.h ./test_ClassyC_All.c

TESTS = test_ClassyC_All.c

all: tests

tests: $(TESTS)
	$(CC) $(CFLAGS) -o run_tests $(TESTS) $(UNITY_SRC) ../ClassyC.h
	./run_tests

clean:
	rm -f run_tests