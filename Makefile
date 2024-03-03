CC=gcc
CFLAGS=-I. -Iinclude
SRC=src/zob.c
TEST_SRC=tests/test_zob.c
OBJ=$(SRC:.c=.o)
TEST_OBJ=$(TEST_SRC:.c=.o)

# Compile and run as per the usual
all: $(OBJ)
	$(CC) $(CFLAGS) -o zob $(OBJ)

# Compile and run tests
test: CFLAGS += -DTESTING
test: $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o test_test $(TEST_OBJ) $(filter-out src/main.o, $(OBJ))
	./test_test

# Generic rule for compiling .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o tests/*.o zob test_test

.PHONY: all test clean
