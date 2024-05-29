#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
     TOKEN_TEXT,
     TOKEN_HEADER1,
     TOKEN_HEADER2,
     TOKEN_BOLD,
     TOKEN_ITALIC,
     TOKEN_LINK,
     TOKEN_LIST_ITEM,
     TOKEN_CODE_BLOCK,
     TOKEN_NEWLINE
} TokenType;

typedef struct Token {
     TokenType type;
     char *content;
     struct Token *next;
} Token;

Token *createToken(TokenType type, const char *content) {
     Token *token = malloc(sizeof(Token));
     if (token == NULL) return NULL;
     token->type = type;
     token->content = strdup(content);
     if (token->content == NULL) {
          free(token);
          return NULL;
     }
     token->next = NULL;
     return token;
}

void freeTokens(Token *token) {
     while (token) {
          Token *next = token->next;
          free(token->content);
          free(token);
          token = next;
     }
}

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

Token *tokenizeMarkdown(const char *markdown) {
     Token *head = NULL;
     Token **current = &head;
     bool insideCodeBlock = false;

     for (const char *ptr = markdown; *ptr;) {
          if (insideCodeBlock) {
               if (strncmp(ptr, "```", 3) == 0) {
                    ptr += 3;
                    insideCodeBlock = false;
                    continue;
               }
               const char *start = ptr;
               while (*ptr && strncmp(ptr, "```", 3) != 0) ptr++;
               *current = createToken(TOKEN_CODE_BLOCK, strndup(start, ptr - start));
               if (!*current) return head;
               current = &(*current)->next;
               continue;
          }

          if (strncmp(ptr, "```", 3) == 0) {
               ptr += 3;
               insideCodeBlock = true;
               continue;
          }

          if (*ptr == '[') {
               ptr++;
               const char *text_start = ptr;
               while (*ptr && *ptr != ']') ptr++;
               char *link_text = strndup(text_start, ptr - text_start);
               ptr++; /* Skip the closing ] */
               ptr++; /* Skip the opening ( */
               const char *url_start = ptr;
               while (*ptr && *ptr != ')') ptr++;
               char *url = strndup(url_start, ptr - url_start);
               ptr++; /* Skip the closing ) */

               char *full_link = malloc(strlen(link_text) + strlen(url) + 32);
               sprintf(full_link, "\\href{%s}{%s}", url, link_text);
               *current = createToken(TOKEN_LINK, full_link);
               free(link_text);
               free(url);
               free(full_link);
               if (!*current) return head;
               current = &(*current)->next;
               continue;
          }

          const char *start = ptr;
          TokenType type = TOKEN_TEXT;
          size_t length = 0;

          if (*ptr == '\n') {
               ptr++;
               continue;
          }

          if (strncmp(ptr, "# ", 2) == 0) {
               ptr += 2;
               start = ptr;
               type = TOKEN_HEADER1;
               while (*ptr && *ptr != '\n') ptr++;
               length = ptr - start;
          } else if (strncmp(ptr, "## ", 3) == 0) {
               ptr += 3;
               start = ptr;
               type = TOKEN_HEADER2;
               while (*ptr && *ptr != '\n') ptr++;
               length = ptr - start;
          } else if (strncmp(ptr, "**", 2) == 0) {
               ptr += 2;
               start = ptr;
               type = TOKEN_BOLD;
               while (*ptr && strncmp(ptr, "**", 2) != 0) ptr++;
               length = ptr - start;
               ptr += 2; /* Skip the closing ** */
          } else if (*ptr == '*') {
               ptr++;
               start = ptr;
               type = TOKEN_ITALIC;
               while (*ptr && *ptr != '*') ptr++;
               length = ptr - start;
               ptr++; /* Skip the closing * */
          } else if (strncmp(ptr, "- ", 2) == 0) {
               ptr += 2;
               start = ptr;
               type = TOKEN_LIST_ITEM;
               while (*ptr && *ptr != '\n') ptr++;
               length = ptr - start;
          } else {
               start = ptr;
               while (*ptr && *ptr != '\n' && *ptr != '*' && strncmp(ptr, "**", 2) != 0 &&
                      *ptr != '[' && strncmp(ptr, "```", 3) != 0 && strncmp(ptr, "- ", 2) != 0 &&
                      *ptr != '#')
                    ptr++;
               length = ptr - start;
          }

          *current = createToken(type, strndup(start, length));
          if (!*current) return head;
          current = &(*current)->next;

          if (*ptr) ptr++; /* Move past the last processed character */
     }

     return head;
}

char *convertTokensToLatex(Token *tokens) {
     size_t totalLength = 0;
     Token *token;

     for (token = tokens; token; token = token->next) {
          switch (token->type) {
               case TOKEN_HEADER1:
               case TOKEN_HEADER2:
               case TOKEN_BOLD:
               case TOKEN_ITALIC:
               case TOKEN_LINK:
               case TOKEN_LIST_ITEM:
               case TOKEN_CODE_BLOCK:
               case TOKEN_TEXT:
               case TOKEN_NEWLINE:
                    totalLength += strlen(token->content) + 50;
                    break;
          }
     }

     /* +1 for null terminator */
     char *latex = malloc(totalLength + 1);
     if (!latex) return NULL;
     char *current = latex;
     bool insideList = false;

     for (token = tokens; token; token = token->next) {
          if (current != latex &&
              (token->type == TOKEN_HEADER1 || token->type == TOKEN_HEADER2 ||
               token->type == TOKEN_LIST_ITEM || token->type == TOKEN_CODE_BLOCK ||
               token->type == TOKEN_LINK || token->type == TOKEN_NEWLINE)) {
               current += sprintf(current, "\n");
          }

          switch (token->type) {
               case TOKEN_HEADER1:
               case TOKEN_HEADER2:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    current += sprintf(current,
                                       token->type == TOKEN_HEADER1 ? "\n\\section{%s}\n\n"
                                                                    : "\n\\subsection{%s}\n\n",
                                       token->content);
                    break;
               case TOKEN_BOLD:
                    current += sprintf(current, "\\textbf{%s}", token->content);
                    break;
               case TOKEN_ITALIC:
                    current += sprintf(current, "\\textbf{%s}", token->content);
                    break;
               case TOKEN_LINK:
                    current += sprintf(current, "%s", token->content);
                    break;
               case TOKEN_LIST_ITEM:
                    if (!insideList) {
                         current += sprintf(current, "\\begin{itemize}\n");
                         insideList = true;
                    }
                    current += sprintf(current, "     \\item %s", token->content);
                    if (token->next && token->next->type != TOKEN_LIST_ITEM) {
                         current += sprintf(current, "\n\\end{itemize}\n");
                         insideList = false;
                    }
                    break;
               case TOKEN_CODE_BLOCK:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    current +=
                        sprintf(current, "\\begin{verbatim}\n%s\\end{verbatim}", token->content);
                    break;
               case TOKEN_NEWLINE:
                    current += sprintf(current, "\n");
                    break;
               case TOKEN_TEXT:
                    current += sprintf(current, "%s", token->content);
                    break;
          }
     }

     if (insideList) {
          current += sprintf(current, "\\end{itemize}\n");
     }

     /* Null terminate the string */
     *current = '\0';
     return latex;
}

void runTex(int argc, char **argv) {
     const char *filePath = NULL;

     if (argc == 2 && strcmp(argv[1], "tex") == 0) {
          char filename[256];
          printf("Enter the markdown filename: ");
          if (fgets(filename, sizeof(filename), stdin) != NULL) {
               filename[strcspn(filename, "\n")] = '\0';
               filePath = filename;
          } else {
               fprintf(stderr, "Failed to read filename.\n");
               return;
          }
     } else if (argc > 2) {
          filePath = argv[2];
     }

     if (!filePath) {
          fprintf(stderr, "No filename provided.\n");
          return;
     }

     char *markdownContent = readFileIntoString(filePath);
     if (markdownContent == NULL) {
          fprintf(stderr, "Failed to read file: %s\n", filePath);
          return;
     }

     Token *tokens = tokenizeMarkdown(markdownContent);
     if (!tokens) {
          fprintf(stderr, "Failed to tokenize markdown.\n");
          free(markdownContent);
          return;
     }

     char *latexContent = convertTokensToLatex(tokens);
     freeTokens(tokens);
     free(markdownContent);

     if (latexContent) {
          printf("%s\n", latexContent);
          free(latexContent);
     } else {
          fprintf(stderr, "Failed to convert tokens to LaTeX.\n");
     }
}

