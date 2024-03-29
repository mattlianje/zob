CC=gcc
CFLAGS=-I./utils
LIBS=-lsqlite3 -lcurl

#  gcc -o zob zob.c zob_rss.c zob_todo.c utils/db_utils.c -lsqlite3 -lcurl -I./utils
#  clang-format -style="{BasedOnStyle: google, IndentWidth: 5, ColumnLimit: 100}" -i zob_tex.c
SRC=$(wildcard src/*.c src/utils/*.c)
OBJ=$(SRC:.c=.o)
EXEC=zob

all: $(EXEC)

$(EXEC):
	$(CC) -o $(EXEC) $(SRC) $(CFLAGS) $(LIBS)

clean:
	rm -f src/*.o src/utils/*.o $(EXEC)
