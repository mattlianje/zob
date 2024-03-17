#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <sqlite3.h>

int db_open(const char* filename, sqlite3** db);
int db_execute(sqlite3* db, const char* sql);
int db_query(sqlite3* db, const char* sql, int (*callback)(void*, int, char**, char**), void* data);

#endif // DB_UTILS_H
