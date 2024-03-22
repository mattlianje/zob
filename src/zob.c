#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zob_rss.h"
#include "zob_todo.h"

/* Prototypes */
void runRssProgram();
void runTodoProgram();

void displayZenMenu() {
    printf("\n「Z O B」— Zen Orchestration of Bytes\n\n");
    printf("1. TODO\n");
    printf("2. RSS\n");
    printf("3. LaTeX\n");
    printf("4. Depart on the wind\n\n");
    printf("Choose your path, sage: ");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "rss") == 0) {
            runRssProgram();
            return 0; 
        } 
        else if (strcmp(argv[1], "todo") == 0) {
            runTodoProgram();
            return 0;
        }
        else {
            printf("「Z O B」— A leaf falls: try again\n");
            return 1;
        }
    }

    if (argc < 2) {
        while (1) {
            displayZenMenu();
            int choice;
            scanf("%d", &choice);
            while (getchar() != '\n');

            switch (choice) {
                case 1:
                    runTodoProgram();
                    break;
                case 2:
                    runRssProgram();
                    break;
                case 3:
                    // TODO
                    //runLatexProgram();
                    break;
                case 4:
                    printf("\n「Z O B」— Until our paths cross again...\n");
                    exit(0);
                default:
                    printf("\n「Z O B」— A leaf falls; unknown paths do not beckon.\n");
            }
        }
    }

    return 0;
}

void runTodoProgram() {
    runTodo();
}

void runRssProgram() {
    runRss();
}

/*
void runLatexProgram() {
    runLatex();
}
*/
