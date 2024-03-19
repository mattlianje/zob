#ifndef CONFIG_H
#define CONFIG_H

#define ZOB_DIRECTORY "/zob"
#define ZOB_DB_NAME "zob.db"
#define ZOBMASTER "Matthieu Court"
#define MAX_TODOS 50


/* zob rss */
#define NUM_PUBLICATIONS 3

struct Publication {
    int id;
    const char* name;
    const char* url;
};

static const struct Publication publications[NUM_PUBLICATIONS] = {
    {1, "France 24 - France", "https://www.france24.com/en/france/rss"},
    {2, "The Economist - Science & Technology", "https://www.economist.com/science-and-technology/rss.xml"},
    {3, "The Globe and Mail - World", "https://www.theglobeandmail.com/arc/outboundfeeds/rss/category/world/"}
};


#endif // CONFIG_H
