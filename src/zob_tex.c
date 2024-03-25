#include "zob_tex.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* Prototypes */
void runTex(int argc, char **argv);

char *readFileIntoString(const char *filename) {
     FILE *file = fopen(filename, "r");
     if (file == NULL) {
          fprintf(stderr, "Error opening file: %s\n", filename);
          return NULL;
     }

     fseek(file, 0, SEEK_END);
     long fileSize = ftell(file);
     fseek(file, 0, SEEK_SET);

     char *buffer = malloc(fileSize + 1);
     if (buffer == NULL) {
          fprintf(stderr, "Memory allocation failed\n");
          fclose(file);
          return NULL;
     }

     fread(buffer, 1, fileSize, file);
     buffer[fileSize] = '\0';
     fclose(file);

     return buffer;
}

void processMarkdownLine(const char *line, bool *insideList, char **output) {
     char *out = *output;

     // Titles and subtitles
     if (strncmp(line, "# ", 2) == 0) {
          out += sprintf(out, "\\section{%s}\n\n", line + 2);
     } else if (strncmp(line, "## ", 3) == 0) {
          out += sprintf(out, "\\subsection{%s}\n\n", line + 3);
     }
     // List items
     else if (strncmp(line, "- ", 2) == 0) {
          if (!(*insideList)) {
               out += sprintf(out, "\\begin{itemize}\n");
               *insideList = true;
          }
          out += sprintf(out, "  \\item %s\n", line + 2);
     }
     // Bold and italic
     else {
          // Simple replacement for **bold** and __italic__
          // TODO: Implement the colour boxes
          const char *ptr = line;
          while (*ptr) {
               if (strncmp(ptr, "**", 2) == 0) {
                    out += sprintf(out, "\\textbf{");
                    ptr += 2;  // Skip the marker
                    while (*ptr && strncmp(ptr, "**", 2) != 0) {
                         *out++ = *ptr++;
                    }
                    if (*ptr) ptr += 2;  // Skip the closing
                    *out++ = '}';
               } else if (strncmp(ptr, "__", 2) == 0) {
                    out += sprintf(out, "\\textit{");
                    ptr += 2;  // Skip the marker
                    while (*ptr && strncmp(ptr, "__", 2) != 0) {
                         *out++ = *ptr++;
                    }
                    if (*ptr) ptr += 2;  // Skip the closing
                    *out++ = '}';
               } else {
                    *out++ = *ptr++;
               }
          }
          *out++ = '\n';
     }
     *output = out;
}

char *replaceMarkdownSyntax(const char *markdown) {
     char *output =
         malloc(strlen(markdown) * 5);  // Should be enough for LaTeX syntax
     if (output == NULL) {
          fprintf(stderr, "Memory allocation error.\n");
          return NULL;
     }

     const char *line = markdown;
     char *outPtr = output;
     bool insideList = false;

     while (*line) {
          const char *nextLine = strchr(line, '\n');
          int lineLength = (nextLine) ? (nextLine - line) : strlen(line);

          char *lineBuffer = malloc(lineLength + 1);
          strncpy(lineBuffer, line, lineLength);
          lineBuffer[lineLength] = '\0';

          processMarkdownLine(lineBuffer, &insideList, &outPtr);

          free(lineBuffer);

          if (nextLine)
               line = nextLine + 1;
          else
               break;
     }

     if (insideList) {
          sprintf(outPtr, "\\end{itemize}\n");
     }

     return output;
}

void runTex(int argc, char **argv) {
     char filename[256];

     if (argc < 2) {
          /* Prompt the user for a filename */
          printf("Enter the Markdown file name: ");
          if (scanf("%255s", filename) != 1) {
               fprintf(stderr, "Error reading filename.\n");
               return;
          }

          size_t len = strlen(filename);
          if (len < 3 || strcmp(filename + len - 3, ".md") != 0) {
               fprintf(stderr, "The file must have a '.md' extension.\n");
               return;
          }
     } else {
          /* Using the provided argument as the filename */
          strncpy(filename, argv[2], sizeof(filename));
          filename[sizeof(filename) - 1] = '\0';  // Ensure null-termination

          size_t len = strlen(filename);
          if (len < 3 || strcmp(filename + len - 3, ".md") != 0) {
               fprintf(stderr, "The file must have a '.md' extension.\n");
               return;
          }
     }

     char *markdownContent = readFileIntoString(filename);
     if (markdownContent == NULL) {
          return;
     }

     /* Concatenating LATEX_PRELUDE, converted LaTeX content, and LATEX_END */
     char *latexContent = replaceMarkdownSyntax(markdownContent);
     if (latexContent != NULL) {
          size_t totalLength = strlen(LATEX_PRELUDE) + strlen(latexContent) + strlen(LATEX_END) + 1;
          char *finalContent = malloc(totalLength);
          if (finalContent == NULL) {
               fprintf(stderr, "Memory allocation error.\n");
               free(latexContent);
               free(markdownContent);
               return;
          }

          snprintf(finalContent, totalLength, "%s%s%s", LATEX_PRELUDE, latexContent, LATEX_END);

          printf("%s", finalContent);

          free(finalContent);
          free(latexContent);
     } else {
          fprintf(stderr, "Conversion failed.\n");
     }

     system("clear || cls");
     free(markdownContent);
}

