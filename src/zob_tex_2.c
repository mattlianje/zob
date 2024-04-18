#include <stdio.h>

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

Token *tokenizeMarkdown(const char *markdown) {
     Token *head = NULL;
     Token **current = &head;
     bool insideCodeBlock = false;
     const char *languageStart = NULL;

     for (const char *ptr = markdown; *ptr;) {
          if (insideCodeBlock) {
               if (strncmp(ptr, "```", 3) == 0) {
                    ptr += 3;  // Skip the closing ```
                    insideCodeBlock = false;
                    continue;
               }
               const char *start = ptr;
               while (*ptr && strncmp(ptr, "```", 3) != 0) ptr++;
               *current = createToken(TOKEN_CODE_BLOCK, strndup(start, ptr - start));
               if (*ptr == '\n') ptr++;  // Handle newline after code block
          } else if (strncmp(ptr, "```", 3) == 0) {
               ptr += 3;  // Skip the opening ```
               languageStart = ptr;
               /*
                * TODO: Handle any of the other non lang box keywords
                */
               while (*ptr && *ptr != '\n') ptr++;
               insideCodeBlock = true;
          } else if (*ptr == '[') {
               ptr++;
               const char *text_start = ptr;
               while (*ptr && *ptr != ']') ptr++;
               char *link_text = strndup(text_start, ptr - text_start);
               ptr += 2;  // Skip ']('
               const char *url_start = ptr;
               while (*ptr && *ptr != ')') ptr++;
               char *url = strndup(url_start, ptr - url_start);
               ptr++;  // Skip ')'

               char *full_link = malloc(strlen(link_text) + strlen(url) + 32);
               sprintf(full_link, "\\href{%s}{%s}", url, link_text);
               *current = createToken(TOKEN_LINK, full_link);
               free(link_text);
               free(url);
               free(full_link);
          } else if (strncmp(ptr, "- ", 2) == 0) {
               ptr += 2;  // Skip '- '
               const char *start = ptr;
               while (*ptr && *ptr != '\n') ptr++;
               *current = createToken(TOKEN_LIST_ITEM, strndup(start, ptr - start));
               if (*ptr == '\n') ptr++;
          } else if (strncmp(ptr, "# ", 2) == 0) {
               ptr += 2;
               const char *start = ptr;
               while (*ptr && *ptr != '\n') ptr++;
               *current = createToken(TOKEN_HEADER1, strndup(start, ptr - start));
               if (*ptr == '\n') ptr++;
          } else if (strncmp(ptr, "## ", 3) == 0) {
               ptr += 3;
               const char *start = ptr;
               while (*ptr && *ptr != '\n') ptr++;
               *current = createToken(TOKEN_HEADER2, strndup(start, ptr - start));
               if (*ptr == '\n') ptr++;
          } else if (strncmp(ptr, "**", 2) == 0) {
               ptr += 2;
               const char *start = ptr;
               while (*ptr && strncmp(ptr, "**", 2) != 0) ptr++;
               *current = createToken(TOKEN_BOLD, strndup(start, ptr - start));
               if (*ptr) ptr += 2;
          } else if (*ptr == '*') {
               ptr++;
               const char *start = ptr;
               while (*ptr && *ptr != '*') ptr++;
               *current = createToken(TOKEN_ITALIC, strndup(start, ptr - start));
               if (*ptr) ptr++;
          } else {
               const char *start = ptr;
               while (*ptr && *ptr != '\n' && *ptr != '*' && strncmp(ptr, "**", 2) != 0 &&
                      *ptr != '[' && strncmp(ptr, "```", 3) != 0 && strncmp(ptr, "- ", 2) != 0)
                    ptr++;
               *current = createToken(TOKEN_TEXT, strndup(start, ptr - start));
          }

          if (*current) {
               current = &(*current)->next;
          }
     }

     return head;
}

char *convertTokensToLatex(Token *tokens) {
     size_t totalLength = 0;
     // A bit hacky but we need to position relative to any current list
     bool insideList = false;

     for (Token *token = tokens; token != NULL; token = token->next) {
          // + 50 for more space for LaTeX commands and any additional formatting
          totalLength += strlen(token->content) + 50;
     }

     char *latex = malloc(totalLength);
     if (!latex) return NULL;
     char *current = latex;

     for (Token *token = tokens; token != NULL; token = token->next) {
          switch (token->type) {
               case TOKEN_HEADER1:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    current += sprintf(current, "\\section{%s}\n", token->content);
                    break;
               case TOKEN_HEADER2:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    current += sprintf(current, "\\subsection{%s}\n", token->content);
                    break;
               case TOKEN_BOLD:
                    current += sprintf(current, "\\textbf{%s}", token->content);
                    break;
               case TOKEN_ITALIC:
                    current += sprintf(current, "\\textit{%s}", token->content);
                    break;
               case TOKEN_LINK:
                    // Need to fix because link token content is not already \\href{url} blabla
                    current += sprintf(current, "%s", token->content);
                    break;
               case TOKEN_LIST_ITEM:
                    if (!insideList) {
                         current += sprintf(current, "\\begin{itemize}\n");
                         insideList = true;
                    }
                    current += sprintf(current, "\\item %s\n", token->content);
                    break;
               case TOKEN_CODE_BLOCK:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    current +=
                        sprintf(current, "\\begin{verbatim}\n%s\\end{verbatim}\n", token->content);
                    break;
               case TOKEN_NEWLINE:
                    if (insideList) {
                         current += sprintf(current, "\\end{itemize}\n");
                         insideList = false;
                    }
                    *current++ = '\n';
                    break;
               case TOKEN_TEXT:
                    current += sprintf(current, "%s", token->content);
                    break;
          }
     }

     // Close any begin{itemize} that might still be open
     if (insideList) {
          current += sprintf(current, "\\end{itemize}\n");
     }

     // To ensure null termination
     *current = '\0';
     return latex;
}
