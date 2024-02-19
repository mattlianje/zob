#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


// A man doesn't need more than 100 TODO's in his life
#define MAX_TODO 100

typedef struct {
    char description[256];
    // dueDate always formatted as <int> YYYYMMDD
    int dueDate; 
    int isDone;
} TodoItem;

char zobSpaceRootDir[1024] = "~/zob"; 
char zobMaster[256] = "Matthieu Court";

TodoItem todos[MAX_TODO];
int todoCount = 0;


int main() {
    return 0;
}

void listZobFiles(const char *directory) {
    struct dirent *de;  
    DIR *dr = opendir(directory);

    if (dr == NULL) {  
        printf("Could not open current directory" );
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strstr(de->d_name, ".zob")) {
            printf("%s\n", de->d_name);
        }
    }

    closedir(dr);
}

void parseZobFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filePath);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        if (strstr(line, "** TODO")) {
            printf("%s", line);  
        }
    }

    free(line);
    fclose(file);
}

void addTodoItem(const char* description, int dueDate) {
    if (todoCount >= MAX_TODO) {
        printf("Maximum TODO limit reached.\n");
        return;
    }
    strncpy(todos[todoCount].description, description, sizeof(todos[todoCount].description));
    todos[todoCount].dueDate = dueDate;
    todos[todoCount].isDone = 0;
    todoCount++;
}

void markTodoDone(int index) {
    if (index < 0 || index >= todoCount) {
        printf("Invalid TODO item index.\n");
        return;
    }
    todos[index].isDone = 1;
    printf("TODO item marked as done.\n");
}

void listTodos(int showAll) {
    for (int i = 0; i < todoCount; i++) {
        if (showAll || !todos[i].isDone) {
            printf("%d: %s - %04d-%02d-%02d %s\n", i + 1, todos[i].description,
                   todos[i].dueDate / 10000, (todos[i].dueDate / 100) % 100, todos[i].dueDate % 100,
                   todos[i].isDone ? "[DONE]" : "");
        }
    }
}

void saveTodosCSV(const char* directory) {
    char filePath[1024];
    snprintf(filePath, sizeof(filePath), "%s/.todos.csv", directory); 

    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        printf("Could not open file %s for writing.\n", filePath);
        return;
    }

    fprintf(file, "DueDate,IsDone,Description\n");

    for (int i = 0; i < todoCount; i++) {
        fprintf(file, "%d,%d,\"%s\"\n", todos[i].dueDate, todos[i].isDone, todos[i].description);
    }

    fclose(file);
    printf("TODO items saved to .todos.csv successfully.\n");
}

void loadTodosCSV(const char* directory) {
    char filePath[1024];
    // Builds filepath for todos dotfile in root of zobpsace
    snprintf(filePath, sizeof(filePath), "%s/.todos.csv", directory); 

    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Could not open file %s for reading. Starting with an empty TODO list.\n", filePath);
        return;
    }

    char line[1024];
    // Skip todo file header
    fgets(line, sizeof(line), file); 

    while (fgets(line, sizeof(line), file) != NULL) {
        TodoItem item;
        char* token = strtok(line, ",");
        if (token) item.dueDate = atoi(token);

        token = strtok(NULL, ",");
        if (token) item.isDone = atoi(token);

        token = strtok(NULL, ",");
        if (token) {
            // Strip newlines
            token[strcspn(token, "\n")] = 0; 
            // Remove potential double quotes around description
            if (token[0] == '"') {
                // Removes leading and trailing quotes
                memmove(token, token+1, strlen(token)); 
                token[strlen(token)-1] = 0; 
            }
            strncpy(item.description, token, sizeof(item.description));
        }

        if (todoCount < MAX_TODO) {
            todos[todoCount++] = item;
        }
    }

    fclose(file);
    printf("TODO items loaded from .todos.csv successfully.\n");
}

// Traverse zobspace including nested directories to 
// find and catalogue TODO items in files with .zob
void refreshZobFiles(const char *directory) {
    struct dirent *de;
    DIR *dr = opendir(directory);

    if (dr == NULL) {
        printf("Could not open directory %s\n", directory);
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        if (de->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue; // Ignore . and ..
            snprintf(path, sizeof(path), "%s/%s", directory, de->d_name);
            refreshZobFiles(path); 
        } else if (strstr(de->d_name, ".zob")) {
            char filePath[1024];
            snprintf(filePath, sizeof(filePath), "%s/%s", directory, de->d_name);
            parseZobFile(filePath); 
        }
    }

    closedir(dr);
}

void interactiveMode() {
    int running = 1;
    char input[256];
    char descInput[256];
    int dueDateInput;
    int choice;
    while (running) {
        printf("\nZOB Management System\n");
        printf("1. List TODO Items\n");
        printf("2. Add a TODO Item\n");
        printf("3. Mark a TODO Item as Done\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1:
                // Pass 0 to list only pending TODOs
                listTodos(1); 
                break;
            case 2:
                printf("Enter TODO description: ");
                fgets(descInput, sizeof(descInput), stdin);
                descInput[strcspn(descInput, "\n")] = 0; 

                printf("Enter due date (YYYYMMDD): ");
                fgets(input, sizeof(input), stdin);
                dueDateInput = atoi(input);

                addTodoItem(descInput, dueDateInput);
                break;
            case 3:
                printf("Enter TODO item number to mark as done: ");
                fgets(input, sizeof(input), stdin);
                markTodoDone(atoi(input) - 1); 
                break;
            case 4:
                running = 0;
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}
