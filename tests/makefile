CC ?= gcc
CFLAGS += -I../ -Iunity
UNITY_SRC = unity.c
SRC = ../ClassyC.h ./test_ClassyC_All.c

# conditionally, if the file exists, add it to the SRC variable
ifeq ($(wildcard ../tinycthread/sources/tinycthread.c),../tinycthread/sources/tinycthread.c)
	SRC += ../tinycthread/sources/tinycthread.c
	CFLAGS += -I../tinycthread/sources
else ifeq ($(wildcard ../tinycthread.c),../tinycthread.c)
	SRC += ../tinycthread.c
endif

TESTS = test_ClassyC_All.c

all: tests

tests: $(SRC) $(UNITY_SRC)
	$(CC) $(CFLAGS) -o run_tests $(SRC) $(UNITY_SRC)
	./run_tests

clean:
	rm -f run_tests
