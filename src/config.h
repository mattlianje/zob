#ifndef CONFIG_H
#define CONFIG_H

/**
 * ZOB GLOBAL
 */
#define ZOB_DIRECTORY "/zob"
#define ZOB_DB_NAME "zob.db"
#define ZOBMASTER "Matthieu Court"
#define MAX_TODOS 50

/**
 * ZOB RSS
 */
#define NUM_PUBLICATIONS 3
#define MAX_ARTICLES 20

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


/**
 * ZOB FMT
 */
#define LINTER_CMD_MAX_SIZE 512

static const char *LINTER_MAPPING[][2] = {
    {"c", "clang-format -style=\"{BasedOnStyle: google, IndentWidth: 5, ColumnLimit: 100}\" -i %s"},
};


/**
 * ZOB TEX
 */
static const char* LATEX_PRELUDE =
    "\\documentclass{article}\n"
    "\\usepackage[utf8]{inputenc}\n"
    "\\usepackage{listings}\n"
    "\\usepackage{hyperref}\n"
    "\\usepackage[most]{tcolorbox}\n"
    "\\usepackage{amsmath}\n"
    "\\hypersetup{\n"
    "    colorlinks=true,\n"
    "    linkcolor=blue,\n"
    "    filecolor=magenta,      \n"
    "    urlcolor=cyan,\n"
    "    pdfpagemode=FullScreen,\n"
    "}\n"
    "\\usepackage{color}\n\n"

    "\\definecolor{codegreen}{rgb}{0,0.6,0}\n"
    "\\definecolor{codegray}{rgb}{0.5,0.5,0.5}\n"
    "\\definecolor{codepurple}{rgb}{0.58,0,0.82}\n"
    "\\definecolor{backcolour}{rgb}{0.95,0.95,0.92}\n\n"

    "\\newtcolorbox{definition}{\n"
    "  colback=green!5!white, \n"
    "  colframe=green!75!black, \n"
    "  title=Definition,\n"
    "}\n\n"

    "\\newtcolorbox{example}{\n"
    "  colback=blue!5!white,\n"
    "  colframe=blue!75!black,\n"
    "  title=Example,\n"
    "}\n\n"

    "\\newtcolorbox{theorem}{\n"
    "  colback=red!5!white,\n"
    "  colframe=red!75!black,\n"
    "  title=Theorem,\n"
    "}\n\n"

    "\\newtcolorbox{centralproblem}{\n"
    "  title=Central Problem,\n"
    "}\n\n"

    "\\lstdefinestyle{mystyle}{\n"
    "    backgroundcolor=\\color{backcolour},   \n"
    "    commentstyle=\\color{codegreen},\n"
    "    keywordstyle=\\color{magenta},\n"
    "    numberstyle=\\tiny\\color{codegray},\n"
    "    stringstyle=\\color{codepurple},\n"
    "    basicstyle=\\footnotesize,\n"
    "    breakatwhitespace=false,         \n"
    "    breaklines=true,                 \n"
    "    captionpos=b,                    \n"
    "    keepspaces=true,                 \n"
    "    numbers=left,                    \n"
    "    numbersep=5pt,                  \n"
    "    showspaces=false,                \n"
    "    showstringspaces=false,\n"
    "    showtabs=false,                  \n"
    "    tabsize=2\n"
    "}\n\n"

    "\\lstset{style=mystyle}\n\n"

    "\\title{Proposal: ???}\n"
    "\\author{" ZOBMASTER
    "}\n"
    "\\date{\\today}\n\n"

    "\\begin{document}\n\n"

    "\\maketitle\n\n"

    "\\tableofcontents\n"
    "\\section*{Overview}\n\n"

    "\\begin{itemize}\n"
    "    \\item \\textbf{What:} ???\n"
    "    \\item \\textbf{Why:} ???\n"
    "\\end{itemize}\n\n";

static const char* LATEX_END = "\\end{document}\n";

#endif  // CONFIG_H
