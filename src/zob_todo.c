#include "config.h"
#include "zob_todo.h"
#include "utils/db_utils.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int todo_id;
  /* In format: YYYYMMDD */
  char due_date[9];
  char status[10];
  char title[50];
  char description[256];
} Todo;


/* Initialization */
void constructFullPath();
void initializeGlobals(); 
void setupSigintHandler();

/* Core */
void displayTodoMenu();
void addTodo(sqlite3 *db);
void viewTodosSortedByDate(sqlite3 *db);
void removeTodo(sqlite3 *db);

/* Data handling */
void sortTodos(Todo todos[], int count);
int compareTodosByDate(const void *a, const void *b);
int readTodosFromFile(const char *filePath, Todo todos[], int maxTodos);

/* User interaction */
void waitForEnterKey();

/* Signal Handling */
void handle_sigint(int sig);

char ZOB_DB_PATH[PATH_MAX];

/* main entrypoint */
void runTodo() {
  constructFullPath();
  displayTodoMenu();
}

void constructFullPath() {
  const char *homeDir = getenv("HOME");
  if (!homeDir) {
    fprintf(stderr, "「Z O B」— Cannot find the home directory.\n");
    exit(EXIT_FAILURE);
  }

  snprintf(ZOB_DB_PATH, sizeof(ZOB_DB_PATH), "%s%s/%s", homeDir, ZOB_DIRECTORY,
           ZOB_DB_NAME);
}

/**
 * Interactive mode for managing TODO items.
 * Changes are persisted to the ZOB_DB SQLite
 */
void displayTodoMenu() {
  int choice;

  sqlite3 *db;

  if (db_open(ZOB_DB_PATH, &db) != SQLITE_OK) {
    printf("*** Failed to open database at path: %s\n", ZOB_DB_PATH);
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return;
  }

  while (1) {
    system("clear || cls");

    printf("\n「Z O B」— Zen Org Binder\n\n");
    printf("1. Add\n");
    printf("2. Remove\n");
    printf("3. View\n");
    printf("4. Exit\n\n");
    printf("Choose an option: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1:
      addTodo(db);
      waitForEnterKey();
      break;
    case 2:
      removeTodo(db);
      waitForEnterKey();
      break;
    case 3:
      system("clear || cls");
      viewTodosSortedByDate(db);
      waitForEnterKey();
      break;
    case 4:
      system("clear || cls");
      printf("Exiting「Z O B」...\n");
      return;
    default:
      printf("Invalid option, please try again.\n");
    }
  }
  sqlite3_close(db);
}

/* signal handler for sigint */
void handle_sigint(int sig) {
  printf("\nExiting「Z O B」...\n");
  exit(EXIT_SUCCESS);
}

void setupSigintHandler() {
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Error setting up signal handler");
    exit(EXIT_FAILURE);
  }
}

/* Prompt before clearing buffer */
void waitForEnterKey() {
  printf("\n「Z O B」Inhale... Press ENTER\n");
  while (getchar() != '\n')
    ;
  getchar();
}

/**
 * Fetches the path to the user's home directory from the environment variables.
 *
 * @return The path to the home directory. Returns NULL if the HOME environment
 * variable is not set.
 */
const char *getHomeDirectory() {
  const char *homeDir = getenv("HOME");
  if (!homeDir) {
    printf("「Z O B」— Unable to discern the path home.\n");
    return NULL;
  }
  return homeDir;
}

/* Checks for date format YYYYMMDD: 8 digits */
bool validateDate(const char *date) {
  if (strlen(date) != 8)
    return false;
  for (int i = 0; i < 8; i++) {
    if (date[i] < '0' || date[i] > '9')
      return false;
  }
  return true;
}

/**
 * Adds a new TODO item to the `todos` table in ZOB_DB
 */
void addTodo(sqlite3 *db) {
  const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS Todos ("
                               "todo_id INTEGER PRIMARY KEY, "
                               "due_date TEXT NOT NULL, "
                               "status TEXT NOT NULL, "
                               "title TEXT NOT NULL, "
                               "description TEXT);";

  char *errMsg = NULL;
  int rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create table: %s\n", errMsg);
    sqlite3_free(errMsg);
    return;
  }

  Todo newTodo;

  printf("\n「Z O B」— Zen Org Binder\nReflect on the task's essence: ");
  scanf(" %[^\n]", newTodo.title);

  while (true) {
    printf("Enter the cycle's completion date (YYYYMMDD): ");
    scanf("%8s", newTodo.due_date);
    if (!validateDate(newTodo.due_date)) {
      printf("「Z O B」— A leaf falls; the date is not proper.\n");
    } else {
      break;
    }
  }

  printf("Whisper the task's details into the wind: ");
  scanf(" %[^\n]", newTodo.description);

  strcpy(newTodo.status, "Pending");

  char *sqlInsert = "INSERT INTO Todos (due_date, status, title, description) "
                    "VALUES (?, ?, ?, ?);";

  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return;
  }

  sqlite3_bind_text(stmt, 1, newTodo.due_date, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, newTodo.status, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, newTodo.title, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, newTodo.description, -1, SQLITE_STATIC);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert todo item: %s\n", sqlite3_errmsg(db));
  } else {
    printf("「Z O B」— Your task joins the stream.\n");
  }
  sqlite3_finalize(stmt);
}

/**
 * Prompts user to remove a TODO given an ID from ZOB_DB
 */
void removeTodo(sqlite3 *db) {
  viewTodosSortedByDate(db);

  int todoId;
  char title[256];

  printf("\n「Z O B」— Zen Org Binder\nWhich task has transcended? Enter its "
         "ID: ");
  scanf("%d", &todoId);

  /* Retrieve the task title before deletion */
  const char *sqlSelect = "SELECT title FROM Todos WHERE todo_id = ?;";
  sqlite3_stmt *selectStmt;
  if (sqlite3_prepare_v2(db, sqlSelect, -1, &selectStmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to contemplate the task's existence: %s\n",
            sqlite3_errmsg(db));
    return;
  }

  sqlite3_bind_int(selectStmt, 1, todoId);
  int stepResult = sqlite3_step(selectStmt);
  if (stepResult != SQLITE_ROW) {
    printf("「Z O B」— No task with such ID was found.\n");
    sqlite3_finalize(selectStmt);
    return;
  }
  strncpy(title, (const char *)sqlite3_column_text(selectStmt, 0),
          sizeof(title));
  sqlite3_finalize(selectStmt);

  /* Confirm deletion */
  printf("\n「Z O B」— The task \"%s\" is ready to leave the scroll. Are you "
         "sure? (y/n): ",
         title);
  getchar();
  char confirmation = getchar();
  if (confirmation != 'y' && confirmation != 'Y') {
    printf("「Z O B」— The task remains tethered.\n");
    return;
  }

  /* Delete the task */
  const char *sqlDelete = "DELETE FROM Todos WHERE todo_id = ?;";
  sqlite3_stmt *deleteStmt;
  if (sqlite3_prepare_v2(db, sqlDelete, -1, &deleteStmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare for the task's release: %s\n",
            sqlite3_errmsg(db));
    return;
  }
  sqlite3_bind_int(deleteStmt, 1, todoId);
  if (sqlite3_step(deleteStmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to release the task: %s\n", sqlite3_errmsg(db));
  } else {
    printf("「Z O B」— \"%s\" has been released into the cosmos.\n", title);
  }
  sqlite3_finalize(deleteStmt);
}

/**
 * Callback function used by SQLite to process each row in the query result.
 * Formats and prints the todo item details in a table-like structure.
 */
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  (void)NotUsed;

  printf("| %-4s | %-10s | %-8s | %-20s | %-30s |\n",
         argv[0] ? argv[0] : "NULL",  // ID
         argv[1] ? argv[1] : "NULL",  // Due Date
         argv[2] ? argv[2] : "NULL",  // Status
         argv[3] ? argv[3] : "NULL",  // Title
         argv[4] ? argv[4] : "NULL"); // Description

  return 0;
}

/**
 * Displays all todo items sorted by their due date.
 */
void viewTodosSortedByDate(sqlite3 *db) {
  char *errMsg = NULL;
  int rc;

  rc = db_open(ZOB_DB_PATH, &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return;
  }

  const char *sql = "SELECT todo_id, due_date, status, title, description FROM "
                    "todos ORDER BY due_date ASC;";

  /* clang-format absolutely mangles this */
  printf("\n「Z O B」— Tasks in the cycle's flow:\n");
  printf("---------------------------------------------------------------------"
         "-------------------\n");
  printf("| %-4s | %-10s | %-8s | %-20s | %-30s |\n", "ID", "Due Date",
         "Status", "Title", "Description");
  printf("---------------------------------------------------------------------"
         "-------------------\n");

  rc = sqlite3_exec(db, sql, callback, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to select data: %s\n", errMsg);
    sqlite3_free(errMsg);
  }

  sqlite3_close(db);
}
