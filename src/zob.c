#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/**
 * Prototypes... 
 */
void loadTodos(void);
void interactiveMode(void);
void saveTodos(void);
void addTodoToCsvIfNew(const char* description, int dueDate);
int isTodoNew(const char* description, int dueDate);
void parseZobFile(const char *filePath);
void refreshZobFiles(const char *directory);
void listTodosInteractive(void);
void toggleTodoStatus(int id);
void addNewTodoInteractive(void);

/**
 * A man doesn't need more than 100 TODO's
 */
#define MAX_TODO 100

typedef struct {
    int id; // Unique ID per TODO
    char description[256]; // Keep it short and concise
    int dueDate; // Always in YYYYMMDD
    char status[10];  // "todo" by default or "done"
} TodoItem;


/*
 * Globals
 */
char zobSpaceRootDir[1024] = "~/zob"; 
char zobMaster[256] = "Matthieu Court";
TodoItem todos[MAX_TODO];
/**
 * Tracks number of items in the TODO buffer
 */
int todoCount = 0; 

#ifndef TESTING
int main() {
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        struct passwd* pw = getpwuid(getuid());
        homeDir = pw->pw_dir;
    }
    snprintf(zobSpaceRootDir, sizeof(zobSpaceRootDir), "%s/zob", homeDir);

    loadTodos(); 
    interactiveMode(); 
    saveTodos(); 
    return 0;
}
#endif

/**
 * Parses a given .zob file and prints lines containing TODO items.
 * adds these lines to todos dotfile
 *
 * @param filePath The path to the .zob file to be parsed.
 */
void parseZobFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filePath);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int dueDate;

    while ((read = getline(&line, &len, file)) != -1) {
        if (strstr(line, "** TODO")) {
            // Format "** TODO [dueDate] description"
            char* start = strstr(line, "[") + 1;
            char* end = strstr(line, "]");
            if (start && end && (end > start)) {
                char dueDateStr[9] = {0}; 
                strncpy(dueDateStr, start, end - start);
                dueDate = atoi(dueDateStr);

                char description[256];
                strncpy(description, end + 2, sizeof(description) - 1); 
                description[sizeof(description) - 1] = '\0'; 

                addTodoToCsvIfNew(description, dueDate);
            }
        }
    }

    free(line);
    fclose(file);
}

/**
 * Adds a new TODO item to the .todos.csv file if it doesn't already exist.
 * The function checks if the TODO item is new by calling `isTodoNew`.
 * If the item is new, it appends the TODO item to the .todos.csv file.
 *
 * @param description The description of the TODO item.
 * @param dueDate The due date of the TODO item, formatted as YYYYMMDD.
 */
void addTodoToCsvIfNew(const char* description, int dueDate) {
    if (!isTodoNew(description, dueDate)) {
        return; 
    }

    // Append new TODO to .todos.csv
    FILE* file = fopen(".todos.csv", "a"); 
    if (file != NULL) {
        fprintf(file, "%d,\"%s\"\n", dueDate, description);
        fclose(file);
    }
}

/**
 * Checks if a given TODO item is new (i.e., it does not already exist in the
 * .todos.csv file).  The function opens the .todos.csv file and searches for a
 * TODO with the same description and due date.  If such a TODO is found, it is
 * considered not new.
 *
 * @param description The description of the TODO item to check.
 * @param dueDate The due date of the TODO item to check, formatted as YYYYMMDD.
 * @return 1 if the TODO item is new, 0 otherwise.
 */
int isTodoNew(const char* description, int dueDate) {
    FILE* file = fopen(".todos.csv", "r");
    if (file == NULL) {
        return 1;
    }

    char line[512];
    char dueDateStr[12]; // Buffers dueDate as a string
    sprintf(dueDateStr, "%d", dueDate); // Converts dueDate to a string

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, description) && strstr(line, dueDateStr)) { 
            fclose(file);
            return 0; // TODO exists
        }
    }

    fclose(file);
    return 1; // TODO is new
}

/**
 * Recursively traverses the specified directory and its subdirectories
 * to find and process `.zob` files. Each `.zob` file found is parsed
 * to extract TODO items, which are then added to the `.todos.csv` file
 * in the zobspace root directory if not already present.
 *
 * @param directory Path to the directory to start the search from.
 */
void refreshZobFiles(const char *directory) {
    struct dirent *de;
    DIR *dr = opendir(directory);

    if (dr == NULL) {
        printf("Could not open directory %s\n", directory);
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        // Checks if the directory entry is a directory
        if (de->d_type == DT_DIR) {
            char path[1024];
            // Skip the current and parent directory entries '.' and '..'
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;

            // Full path of subdirectory
            snprintf(path, sizeof(path), "%s/%s", directory, de->d_name);
            refreshZobFiles(path);

        } else if (strstr(de->d_name, ".zob")) {

            // Construct the full path of the .zob file
            char filePath[1024];
            snprintf(filePath, sizeof(filePath), "%s/%s", directory, de->d_name);

            // Parse - find and write the TODO s
            parseZobFile(filePath);
        }
    }

    closedir(dr);
}

/**
 * Loads TODO items from the .todos.csv file into the global todos array.
 * Initializes todoCount based on the number of items loaded.
 * Each line from the file is expected to follow the format: description,dueDate,status
 */
void loadTodos() {
    char filePath[1024];
    snprintf(filePath, sizeof(filePath), "%s/.todos.csv", zobSpaceRootDir);
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf("No TODO items found.\n");
        return;
    }
    char line[512];
    todoCount = 0;
    while (fgets(line, sizeof(line), file) && todoCount < MAX_TODO) {
        // !! : CSV format must be description,dueDate,status
        sscanf(line, "\"%[^\"]\",%d,%[^\"]", todos[todoCount].description, &todos[todoCount].dueDate, todos[todoCount].status);
        // Sequential ID incrementing
        todos[todoCount].id = todoCount + 1; 
        todoCount++;
    }
    fclose(file);
}

int countTodos(void) {
    return todoCount; 
}

/**
 * Saves all TODO items from the global todos array to the .todos.csv file.
 * Overwrites the existing file with updated content.
 */
void saveTodos() {
    char filePath[1024];
    snprintf(filePath, sizeof(filePath), "%s/.todos.csv", zobSpaceRootDir);
    FILE *file = fopen(filePath, "w");
    if (!file) {
        printf("Failed to open .todos.csv for writing.\n");
        return;
    }
    for (int i = 0; i < todoCount; i++) {
        fprintf(file, "\"%s\",%d,%s\n", todos[i].description, todos[i].dueDate, todos[i].status);
    }
    fclose(file);
}

/**
 * Lists all TODO items currently loaded in the global todos array.
 * Displays each item's ID, description, due date, and status.
 */
void listTodosInteractive() {
    for (int i = 0; i < todoCount; i++) {
        printf("%d: %s - Due: %d [%s]\n", todos[i].id, todos[i].description, todos[i].dueDate, todos[i].status);
    }
}

/**
 * Toggles the status of a TODO item identified by its ID.
 * Changes the status from "todo" to "done" or vice versa.
 * If the specified ID is not found, prints an error message.
 * @param id The ID of the TODO item to toggle the status of.
 */
void toggleTodoStatus(int id) {
    for (int i = 0; i < todoCount; i++) {
        if (todos[i].id == id) {
            if (strcmp(todos[i].status, "todo") == 0) {
                strcpy(todos[i].status, "done");
            } else {
                strcpy(todos[i].status, "todo");
            }
            printf("TODO %d marked as %s.\n", id, todos[i].status);
            return;
        }
    }
    printf("TODO ID %d not found.\n", id);
}

/**
 * Prompts the user for a description and due date, then adds a new TODO item.
 * The new item is assigned the next sequential ID and a default status of "todo".
 * If the maximum number of TODOs (MAX_TODO) is reached, displays an error message.
 */
void addNewTodoInteractive() {
    if (todoCount >= MAX_TODO) {
        printf("Maximum number of TODOs reached.\n");
        return;
    }

    char description[256];
    int dueDate;

    printf("Enter TODO description: ");
    fgets(description, sizeof(description), stdin);
    description[strcspn(description, "\n")] = 0; 

    printf("Enter due date (YYYYMMDD): ");
    scanf("%d", &dueDate);
    getchar(); 

    // Add the new TODO
    strcpy(todos[todoCount].description, description);
    todos[todoCount].dueDate = dueDate;
    strcpy(todos[todoCount].status, "todo"); 
    todos[todoCount].id = todoCount + 1; 
    todoCount++;

    printf("New TODO added.\n");
}

/**
 * Interactive mode for managing TODO items.
 * Changes are saved to the .todos.csv file upon exiting.
 */
void interactiveMode() {
    int running = 1;
    char input[256];
    int choice;

    while (running) {
        printf("\n「Z O B」— Zen Org Binder\n");
        printf("1. List TODO Items\n");
        printf("2. Add a TODO Item\n");
        printf("3. Mark a TODO Item as Done\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1:
                listTodosInteractive();
                break;
            case 2:
                addNewTodoInteractive();
                break;
            case 3:
                printf("Enter TODO ID to toggle status: ");
                fgets(input, sizeof(input), stdin);
                int id = atoi(input);
                toggleTodoStatus(id);
                break;
            case 4:
                running = 0;
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    saveTodos(); 
}
