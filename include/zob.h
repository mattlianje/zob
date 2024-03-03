#ifndef ZOB_H
#define ZOB_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

typedef struct {
    int id;
    char description[256];
    int dueDate;
    char status[10];
} TodoItem;

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
int countTodos(void);

#endif // ZOB_H
