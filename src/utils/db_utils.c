#include "db_utils.h"
#include <stdio.h>

/**
 * Opens a connection to an SQLite database.
 *
 * @param filename The path to the database file.
 * @param db A pointer to an sqlite3* variable.
 * @return SQLITE_OK on success, or an SQLite error code ...
 */
int db_open(const char *filename, sqlite3 **db) {
  return sqlite3_open(filename, db);
}

/**
 * Executes an SQL statement on an open SQLite database.
 *
 * @param db A pointer to an open SQLite database.
 * @param sql String literal of SQL statement.
 * @return SQLITE_OK on successful execution or SQLite error code ...
 *
 * This function can be used for SQL statements that do not return data (e.g.,
 * CREATE, INSERT, UPDATE).
 */
int db_execute(sqlite3 *db, const char *sql) {
  char *errMsg = NULL;
  int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
  return rc;
}

/**
 * Executes an SQL query on an open SQLite database and processes the results.
 *
 * @param db A pointer to an open SQLite database.
 * @param sql String literal of SQL statement.
 * @param callback A callback function that will be invoked for each row in the
 * query result.
 * @param data A pointer to user data that will be passed to the callback
 * function.
 * @return SQLITE_OK on successful execution, or an SQLite error code on
 * failure.
 *
 * The callback function should match the signature:
 * int callback(void *data, int numColumns, char **fieldValues, char
 * **columnNames);
 *   - data: user data provided in the db_query call.
 *   - numColumns: the number of columns in the result.
 *   - fieldValues: an array of strings representing the values of each field in
 * the row.
 *   - columnNames: an array of strings representing the names of each column in
 * the result set.
 */
int db_query(sqlite3 *db, const char *sql,
             int (*callback)(void *, int, char **, char **), void *data) {
  char *errMsg = NULL;
  int rc = sqlite3_exec(db, sql, callback, data, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
  return rc;
}
