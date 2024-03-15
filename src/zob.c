#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * CSV Schema Reminder: The .todos.csv file follows this structure:
 * todo_id (integer), due_date (YYYYMMDD as string), status (string), title
 * (string), description (string)
 */
const char *zob_directory = "~/zob";
const char *zobmaster = "Matthieu Court";
/* A man doesn't need more than 50 todos in his life */
int max_todos = 100;

typedef struct {
  int todo_id;
  char due_date[9]; // YYYYMMdd format
  char status[10];
  char title[50];
  char description[256];
} Todo;

void displayMenu();
void addTodo();
void removeTodo();
int generateTodoId();
void initializeGlobals();
void viewTodosSortedByDate();
int compareTodosByDate(const void *a, const void *b);
void sortTodos(Todo todos[], int count);
int readTodosFromFile(const char *filePath, Todo todos[], int maxTodos);
void setupSigintHandler();
void waitForEnterKey();
void handle_sigint(int sig);

int main() {
  initializeGlobals();
  displayMenu();
  return 0;
}

/**
 * Interactive mode for managing TODO items.
 * Changes are saved to the .todos.csv file upon exiting.
 */
void displayMenu() {
  int choice;

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
      addTodo();
      waitForEnterKey();
      break;
    case 2:
      removeTodo();
      waitForEnterKey();
      break;
    case 3:
      system("clear || cls");
      viewTodosSortedByDate();
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
}

/* Initializes zobmaster and zobspace */
void initializeGlobals() {
  const char *homeDir = getenv("HOME");
  if (!homeDir) {
    printf("「Z O B」— Path to dwelling unknown.\n");
    exit(EXIT_FAILURE);
  }

  static char zobDirBuffer[PATH_MAX];
  snprintf(zobDirBuffer, sizeof(zobDirBuffer), "%s/zob", homeDir);
  zob_directory = zobDirBuffer;
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
  // Fetch the HOME environment variable
  const char *homeDir = getenv("HOME");
  if (!homeDir) {
    // HOME is not set; handle as needed
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
 * Adds a new TODO item to the .todos.csv file.
 * Prompts the user for the item's details, generates a unique ID,
 * and saves the new entry. Ensures the due date format is validated
 * before saving. The new item is appended to the file.
 */
void addTodo() {
  char filePath[1024];
  snprintf(filePath, sizeof(filePath), "%s/todos.csv", zob_directory);

  Todo newTodo;
  newTodo.todo_id = generateTodoId();
  if (newTodo.todo_id == -1)
    return;

  printf("\n「Z O B」— Zen Org Binder\nReflect on the task's essence: ");
  scanf(" %[^\n]", newTodo.title);

  while (true) {
    printf("Enter the cycle's completion date (YYYYMMDD): ");
    scanf("%8s", newTodo.due_date);
    if (!validateDate(newTodo.due_date)) {
      printf("「Z O B」— A leaf falls; the date is not proper.\n");
    } else
      break;
  }

  printf("Whisper the task's details into the wind: ");
  scanf(" %[^\n]", newTodo.description);

  strcpy(newTodo.status, "Pending");

  FILE *file = fopen(filePath, "a");
  if (!file) {
    printf("「Z O B」— Unable to open the scroll.\n");
    return;
  }

  fprintf(file, "%d,%s,%s,%s,%s\n", newTodo.todo_id, newTodo.due_date,
          newTodo.status, newTodo.title, newTodo.description);
  fclose(file);
  printf("「Z O B」— Your task joins the stream.\n");
}

/**
 * Removes a specified TODO item by its ID from the .todos.csv file.
 * Users are prompted to enter the ID of the TODO they wish to remove.
 * The function ensures only the specified TODO is removed, preserving all
 * others.
 */
void removeTodo() {
  char filePath[PATH_MAX], tempFilePath[PATH_MAX];
  snprintf(filePath, sizeof(filePath), "%s/todos.csv", zob_directory);
  snprintf(tempFilePath, sizeof(tempFilePath), "%s/temp_todos.csv",
           zob_directory);

  FILE *file = fopen(filePath, "r");
  FILE *tempFile = fopen(tempFilePath, "w");

  if (!file || !tempFile) {
    perror("「Z O B」— Failed to open the scroll");
    exit(EXIT_FAILURE);
  }

  printf("\n「Z O B」— Zen Org Binder\n");
  printf("Which task has reached enlightenment? Enter its ID: ");
  int removeId;
  scanf("%d", &removeId);

  char line[1024];
  int found = 0;

  while (fgets(line, sizeof(line), file)) {
    int todoId;
    // todo ID first field in the CSV
    if (sscanf(line, "%d,", &todoId) == 1 && todoId == removeId) {
      found = 1; // Mark as found, but don't write to temp file
      continue;
    }
    fputs(line, tempFile);
  }

  fclose(file);
  fclose(tempFile);

  // Update the file if todo found
  if (found) {
    remove(filePath);
    rename(tempFilePath, filePath);
    printf("「Z O B」— The task with ID %d is gone.\n", removeId);
  } else {
    remove(tempFilePath);
    printf("「Z O B」— No task with such ID was found.\n");
  }
}

/**
 * Generates a unique ID for a new TODO item.
 * The ID is determined by finding the highest existing ID in .todos.csv and
 * incrementing it. If the maximum number of TODOs is reached, the function
 * returns -1 to indicate failure.
 */
int generateTodoId() {
  char filePath[1024];
  snprintf(filePath, sizeof(filePath), "%s/todos.csv", zob_directory);

  FILE *file = fopen(filePath, "r");
  if (!file) {
    printf("「Z O B」— Pathway obscured, cannot find todos.csv\n");
    return -1;
  }

  int maxId = 0, todoCount = 0;
  char line[1024];

  while (fgets(line, sizeof(line), file)) {
    int id;
    if (sscanf(line, "%d,", &id) == 1) {
      maxId = id > maxId ? id : maxId;
      todoCount++;
    }
  }

  fclose(file);

  if (todoCount >= max_todos) {
    printf("「Z O B」— The scroll is full, no more todos can be inscribed\n");
    return -1;
  }

  return maxId + 1;
}

/**
 * Displays todo items sorted by their due date from the .todos.csv file.
 */
void viewTodosSortedByDate() {
  Todo todos[max_todos];
  char filePath[1024];
  snprintf(filePath, sizeof(filePath), "%s/todos.csv", zob_directory);

  int count = readTodosFromFile(filePath, todos, max_todos);
  if (count == -1) {
    printf("「Z O B」— Unable to conjure the list of tasks.\n");
    return;
  }
  // Sort todos by date
  sortTodos(todos, count);

  // Display sorted todos
  printf("\n「Z O B」— Tasks in the cycle's flow:\n");
  for (int i = 0; i < count; ++i) {
    printf("\nTask %d: [Due: %s] %s\n", todos[i].todo_id, todos[i].due_date,
           todos[i].title);
    printf("Status: %s\n", todos[i].status);
    printf("Detail: %s\n", todos[i].description);
    printf("\n---\n"); // Simple delimiter between tasks
  }
}

/**
 * Compares two todo items based on their due date for sorting.
 */
int compareTodosByDate(const void *a, const void *b) {
  const Todo *todoA = (const Todo *)a;
  const Todo *todoB = (const Todo *)b;
  return strcmp(todoA->due_date, todoB->due_date);
}

/**
 * Sorts an array of todo items by due date using the quicksort algorithm.
 */
void sortTodos(Todo todos[], int count) {
  qsort(todos, count, sizeof(Todo), compareTodosByDate);
}

/**
 * Reads todo items from a specified .todos.csv file into an array of Todo
 * structures. Returns the number of todos read.
 */
int readTodosFromFile(const char *filePath, Todo todos[], int maxTodos) {
  FILE *file = fopen(filePath, "r");
  if (!file)
    return -1;

  char line[1024];
  int count = 0;

  while (fgets(line, sizeof(line), file) && count < maxTodos) {
    sscanf(line, "%d,%8s,%[^,],%[^,],%255[^\n]", &todos[count].todo_id,
           todos[count].due_date, todos[count].status, todos[count].title,
           todos[count].description);
    count++;
  }

  fclose(file);
  return count;
}
