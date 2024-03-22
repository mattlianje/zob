#include "config.h"
#include <ctype.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * A structure to hold data received from an HTTP response in memory. This
 * structure is used with libcurl's write callback function to dynamically
 * accumulate response data.
 *
 * @param memory Pointer to the buffer that holds the received data. This buffer
 * is dynamically allocated and resized as needed.
 * @param size The current size of the buffer, in bytes. This value is updated
 * as more data is written to the buffer.
 */
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
void displayMenu();

int main() {
  int choice;

  while (1) {
    system("clear || cls");
    displayMenu();
    scanf("%d", &choice);
    while (getchar() != '\n')
      ;

    if (choice > 0 && choice <= NUM_PUBLICATIONS) {
      httpGet(publications[choice - 1].url);
      printf("\nPress ENTER to return to the menu...");
      getchar();
    } else if (choice == NUM_PUBLICATIONS + 1) {
      printf("「Z O B」— Exiting... May your path be enlightened.\n");
      break;
    } else {
      printf("「Z O B」— A leaf falls; the choice is unknown. Please select "
             "again.\n");
    }
  }
  return 0;
}

void displayMenu() {
  printf("\n「Z O B」— Zen RSS\n\n");
  for (int i = 0; i < NUM_PUBLICATIONS; ++i) {
    printf("%d. %s\n", i + 1, publications[i].name);
  }
  printf("%d. Exit\n\n", NUM_PUBLICATIONS + 1);
  printf("Select the source or exit: ");
}

/**
 * Trims leading and trailing whitespace from a string, removes CDATA sections,
 * and returns the trimmed string.
 * @param str A pointer to the string to be trimmed.
 * @return A pointer to the trimmed string.
 */
char *trimWhitespace(char *str) {
  char *end;
  while (isspace((unsigned char)*str))
    str++;

  if (*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  end[1] = '\0';

  // Remove CDATA
  char *cdataStart = strstr(str, "<![CDATA[");
  if (cdataStart) {
    char *cdataEnd = strstr(str, "]]>");
    if (cdataEnd && cdataEnd > cdataStart) {
      cdataStart += 9; // Length of the CDATA tag
      size_t newLength = (cdataEnd - cdataStart);
      memmove(str, cdataStart, newLength);
      str[newLength] = '\0';
    }
  }
  return str;
}

/**
 * Reallocates memory to store data received from a libcurl operation, ensuring
 * the memory block grows as needed.
 * @param contents Pointer to the received data.
 * @param size Size of one data element received.
 * @param nmemb Number of data elements received.
 * @param userp Pointer to a user-defined struct (MemoryStruct) where the
 * received data is stored.
 * @return The total size of the received data.
 */
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

/**
 * Parses the given RSS content, extracting and formatting the title, link,
 * description, and publication date of each item. The formatted output is
 * printed to stdout.
 *
 * @param rss_content A string containing the complete RSS feed content to be
 * parsed.
 */
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

    char title[512] = {0}, link[512] = {0}, description[1024] = {0},
         pubDate[512] = {0}, dateFormatted[20] = {0};

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
      snprintf(dateFormatted, sizeof(dateFormatted), "%d %s %d", day, month,
               year);
    }

    printf("#%-5d\033[1m\033[36m「%s」\033[0m \033[32m%s\033[0m \n\t\t"
           "%s\n\033[34m\t\t%s\033[0m\n\n",
           itemCount++, trimWhitespace(title), dateFormatted,
           trimWhitespace(description), trimWhitespace(link));

    itemStart = descriptionEnd;
  }
}

/**
 * Writes data received from a libcurl request to the provided stream. This
 * function is intended to be used as a callback with CURLOPT_WRITEFUNCTION in
 * libcurl operations.
 *
 * @param ptr Pointer to the data received from the libcurl request.
 * @param size Size of each element to be written.
 * @param nmemb Number of elements to be written.
 * @param stream A FILE stream where the data will be written.
 * @return The total number of bytes written.
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

/**
 * Performs an HTTP GET request to the specified URL using libcurl, and
 * processes the response using the parse_rss function. The MemoryStruct
 * structure is used to dynamically store the response data, which is then
 * parsed to extract RSS feed items.
 *
 * @param url The URL from which to fetch RSS content.
 */
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
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  } else {
    parse_rss(chunk.memory);
  }

  free(chunk.memory);
  curl_easy_cleanup(curl);
}
