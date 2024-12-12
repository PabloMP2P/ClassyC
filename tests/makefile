ifeq ($(OS),Windows_NT)
    CC = gcc
else
    CC ?= gcc
endif

CFLAGS += -I../ -Iunity -Wall -pedantic -Wextra
UNITY_SRC = unity.c
SRC = ../ClassyC.h ./test_ClassyC_All.c

TESTS = test_ClassyC_All.c

all: tests

tests: $(SRC) $(UNITY_SRC)
	$(CC) $(CFLAGS) -o run_tests $(SRC) $(UNITY_SRC)
	./run_tests

clean:
	rm -f run_tests
