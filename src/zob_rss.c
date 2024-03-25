#include <ctype.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zob_rss.h"

struct MemoryStruct {
     char *memory;
     size_t size;
};

/* Prototypes */
char *trimWhitespace(char *str);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  struct MemoryStruct *mem);
void parse_rss(const char *rss_content);
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
void httpGet(const char *url);
void displayRssMenu();

/* main entrypoint */
void runRss() {
     int choice;

     while (1) {
          system("clear || cls");
          displayRssMenu();
          scanf("%d", &choice);
          while (getchar() != '\n')
               ;

          if (choice > 0 && choice <= NUM_PUBLICATIONS) {
               httpGet(publications[choice - 1].url);
               printf("\nPress ENTER to return to the menu...");
               getchar();
          } else if (choice == NUM_PUBLICATIONS + 1) {
               system("clear || cls");
               printf("「Z O B」— Exiting... May your path be enlightened.\n");
               break;
          } else {
               printf(
                   "「Z O B」— A leaf falls; the choice is unknown. Please select "
                   "again.\n");
          }
     }
}

void displayRssMenu() {
     printf("\n「Z O B」— Zen RSS\n\n");
     for (int i = 0; i < NUM_PUBLICATIONS; ++i) {
          printf("%d. %s\n", i + 1, publications[i].name);
     }
     printf("%d. Exit\n\n", NUM_PUBLICATIONS + 1);
     printf("Select the source or exit: ");
}

char *trimWhitespace(char *str) {
     char *end;
     while (isspace((unsigned char)*str)) str++;

     if (*str == 0) return str;

     end = str + strlen(str) - 1;
     while (end > str && isspace((unsigned char)*end)) end--;

     end[1] = '\0';

     // Remove CDATA
     char *cdataStart = strstr(str, "<![CDATA[");
     if (cdataStart) {
          char *cdataEnd = strstr(str, "]]>");
          if (cdataEnd && cdataEnd > cdataStart) {
               cdataStart += 9;  // Length of the CDATA tag
               size_t newLength = (cdataEnd - cdataStart);
               memmove(str, cdataStart, newLength);
               str[newLength] = '\0';
          }
     }
     return str;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  struct MemoryStruct *mem) {
     size_t realsize = size * nmemb;

     char *ptr = realloc(mem->memory, mem->size + realsize + 1);
     if (!ptr) {
          fprintf(stderr, "not enough memory (realloc returned NULL)\n");
          return 0;
     }

     mem->memory = ptr;
     memcpy(&(mem->memory[mem->size]), contents, realsize);
     mem->size += realsize;
     mem->memory[mem->size] = 0;

     return realsize;
}

void parse_rss(const char *rss_content) {
     int itemCount = 1;
     const char *itemStart = rss_content;
     const char *titleStart, *titleEnd;
     const char *linkStart, *linkEnd;
     const char *descriptionStart, *descriptionEnd;
     const char *pubDateStart, *pubDateEnd;

     while ((itemStart = strstr(itemStart, "<item>"))) {
          itemStart += 6;

          titleStart = strstr(itemStart, "<title>") + 7;
          titleEnd = strstr(titleStart, "</title>");
          linkStart = strstr(itemStart, "<link>") + 6;
          linkEnd = strstr(linkStart, "</link>");
          descriptionStart = strstr(itemStart, "<description>") + 13;
          descriptionEnd = strstr(descriptionStart, "</description>");
          pubDateStart = strstr(itemStart, "<pubDate>") + 9;
          pubDateEnd = strstr(pubDateStart, "</pubDate>");

          char title[512] = {0}, link[512] = {0}, description[1024] = {0}, pubDate[512] = {0},
               dateFormatted[20] = {0};

          strncpy(title, titleStart, titleEnd - titleStart);
          strncpy(link, linkStart, linkEnd - linkStart);
          strncpy(description, descriptionStart, descriptionEnd - descriptionStart);
          strncpy(pubDate, pubDateStart, pubDateEnd - pubDateStart);

          int day, year;
          char month[20];
          /* All the big sites use <pubDate>Thu, 14 Mar 2024 20:25:28 +0000</pubDate>
           * ... I only want DD <Month> YYYY
           */
          if (sscanf(pubDate, "%*[^,], %d %s %d", &day, month, &year) == 3) {
               snprintf(dateFormatted, sizeof(dateFormatted), "%d %s %d", day, month, year);
          }

          printf(
              "#%-5d\033[1m\033[36m「%s」\033[0m \033[32m%s\033[0m \n\t\t"
              "%s\n\033[34m\t\t%s\033[0m\n\n",
              itemCount++, trimWhitespace(title), dateFormatted, trimWhitespace(description),
              trimWhitespace(link));

          itemStart = descriptionEnd;
     }
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
     size_t written = fwrite(ptr, size, nmemb, stream);
     return written;
}

void httpGet(const char *url) {
     CURL *curl = curl_easy_init();
     if (!curl) {
          fprintf(stderr, "Failed to initialize cURL\n");
          return;
     }

     struct MemoryStruct chunk;
     /* Will be grown as needed by the above realloc */
     chunk.memory = malloc(1);
     chunk.size = 0;

     curl_easy_setopt(curl, CURLOPT_URL, url);
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
     curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

     CURLcode res = curl_easy_perform(curl);
     if (res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
     } else {
          parse_rss(chunk.memory);
     }

     free(chunk.memory);
     curl_easy_cleanup(curl);
}

