#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zob_rss.h"
#include "zob_tex.h"
#include "zob_todo.h"
#include "zob_fmt.h"

/* Prototypes */
void runRssProgram();
void runTodoProgram();
void runTexProgram(int argc, char *argv[]); 
void runFmtProgram(int argc, char *argv[]); 

void displayZenMenu() {
     printf("\n「Z O B」— Zen Org Binder\n\n");
     printf("1. Todo\n");
     printf("2. RSS\n");
     printf("3. LaTeX\n");
     printf("4. fmt\n");
     printf("5. Depart on the wind\n\n");
     printf("Choose your path: ");
}

int main(int argc, char *argv[]) {
     if (argc > 1) {
          if (strcmp(argv[1], "rss") == 0) {
               runRssProgram();
               return 0;
          } else if (strcmp(argv[1], "todo") == 0) {
               runTodoProgram();
               return 0;
          } else if (strcmp(argv[1], "tex") == 0) {
               runTexProgram(argc, argv);
               return 0;
          } else if (strcmp(argv[1], "fmt") == 0) {
               runFmtProgram(argc, argv);
               return 0;
          } else {
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
                         runTexProgram(0, NULL);
                         break;
                    case 4:
                         runFmtProgram(0, NULL);
                         break;
                    case 5:
                         system("clear || cls");
                         printf("\n「Z O B」— Until our paths cross again...\n");
                         exit(0);
                    default:
                         printf("\n「Z O B」— A leaf falls; the path in unclear\n");
               }
          }
     }

     return 0;
}

void runTodoProgram() { runTodo(); }

void runRssProgram() { runRss(); }

void runTexProgram(int argc, char *argv[]) { runTex(argc, argv); }

void runFmtProgram(int argc, char *argv[]) { runFmt(argc, argv); }
