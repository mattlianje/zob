#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

// Callback function for libcurl's CURLOPT_WRITEFUNCTION
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    (void)stream; 
    fwrite(ptr, size, nmemb, stdout);
    return size * nmemb;
}

void performHttpGet(const char* url) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

int main() {
    performHttpGet("https://www.france24.com/en/france/rss");

    return 0;
}

