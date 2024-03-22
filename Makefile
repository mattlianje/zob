CC=gcc
CFLAGS=-I./utils
LDFLAGS=-lsqlite3
SRC=zob.c utils/db_utils.c
OBJ=$(SRC:.c=.o)
EXEC=zob-db

.PHONY: all check_sqlite clean

all: check_sqlite $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

#  gcc -o zob-db zob.c utils/db_utils.c -lsqlite3 -I./utils
#  gcc -o zob_rss zob_rss.c -lcurl

check_sqlite:
	@echo "Checking for SQLite3..."
	@command -v sqlite3 >/dev/null 2>&1 || { echo >&2 "SQLite3 is not installed. Please install SQLite3."; \
	brew install sqlite3; \
	exit 1; }

clean:
	rm -f $(EXEC) $(OBJ)
