#include "zob_fmt.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

void runFmt(int argc, char **argv) {
     char filename[256] = {0};

     if (argc < 2) {
          system("clear || cls");
          printf("「Z O B」- Enter the file name: ");
          if (scanf("%255s", filename) != 1) {
               fprintf(stderr, "「Z O B」- Error reading filename.\n");
               return;
          }
     } else {
          strncpy(filename, argv[2], sizeof(filename) - 1);
     }

     const char *fileExtension = strrchr(filename, '.');
     if (!fileExtension) {
          fprintf(stderr, "「Z O B」- File extension not found.\n");
          return;
     }

     bool commandExecuted = false;
     for (int i = 0; i < sizeof(LINTER_MAPPING) / sizeof(LINTER_MAPPING[0]); ++i) {
          if (strcmp(fileExtension + 1, LINTER_MAPPING[i][0]) == 0) {
               system("clear || cls");
               char command[LINTER_CMD_MAX_SIZE];
               snprintf(command, sizeof(command), LINTER_MAPPING[i][1], filename);
               int status = system(command);
               if (status != -1) {
                    printf("「Z O B」- ✨ %s ✨ has been cleansed.\n", filename);
                    commandExecuted = true;
                    break;
               }
          }
     }

     if (!commandExecuted) {
          fprintf(stderr, "「Z O B」- No linter configuration found for the given file type.\n");
     }
}
