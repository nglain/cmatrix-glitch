/*
    cmatrix.c

    Copyright (C) 1999-2017 Chris Allegretta
    Copyright (C) 2017-Present Abishek V Ashok

    This file is part of cmatrix.

    cmatrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cmatrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cmatrix. If not, see <http://www.gnu.org/licenses/>.

*/

#define NCURSES_WIDECHAR 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <locale.h>
#include <wchar.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef EXCLUDE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#ifdef _WIN32
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#elif defined(HAVE_TERMIO_H)
#include <termio.h>
#endif

#ifdef __CYGWIN__
#define TIOCSTI 0x5412
#endif

/* Matrix typedef */
typedef struct cmatrix {
    int val;
    bool is_head;
} cmatrix;

/* Global variables */
int console = 0;
int xwindow = 0;
int lock = 0;
int mathmode = 0;
int waterfall = 0;
char *waterfall_text = NULL;

/* Big font for waterfall mode - DOUBLE WIDTH (10x7 per char) for j+=2 */
const char *font_latin[128] = {
    ['0'] = "0011111100,1100000011,1100000011,1100000011,1100000011,1100000011,0011111100",
    ['1'] = "0000110000,0011110000,0000110000,0000110000,0000110000,0000110000,0011111100",
    ['2'] = "0011111100,1100000011,0000000011,0000111100,0011000000,1100000000,1111111111",
    ['3'] = "0011111100,1100000011,0000000011,0000111100,0000000011,1100000011,0011111100",
    ['4'] = "0000001100,0000111100,0011001100,1100001100,1111111111,0000001100,0000001100",
    ['5'] = "1111111111,1100000000,1111111100,0000000011,0000000011,1100000011,0011111100",
    ['6'] = "0000111100,0011000000,1100000000,1111111100,1100000011,1100000011,0011111100",
    ['7'] = "1111111111,0000000011,0000001100,0000110000,0011000000,0011000000,0011000000",
    ['8'] = "0011111100,1100000011,1100000011,0011111100,1100000011,1100000011,0011111100",
    ['9'] = "0011111100,1100000011,1100000011,0011111111,0000000011,0000001100,0011110000",
    [':'] = "0000000000,0000110000,0000110000,0000000000,0000110000,0000110000,0000000000",
    [' '] = "0000000000,0000000000,0000000000,0000000000,0000000000,0000000000,0000000000",
    ['A'] = "0011111100,1100000011,1100000011,1111111111,1100000011,1100000011,1100000011",
    ['B'] = "1111111100,1100000011,1100000011,1111111100,1100000011,1100000011,1111111100",
    ['C'] = "0011111100,1100000011,1100000000,1100000000,1100000000,1100000011,0011111100",
    ['D'] = "1111111100,1100000011,1100000011,1100000011,1100000011,1100000011,1111111100",
    ['E'] = "1111111111,1100000000,1100000000,1111111100,1100000000,1100000000,1111111111",
    ['F'] = "1111111111,1100000000,1100000000,1111111100,1100000000,1100000000,1100000000",
    ['G'] = "0011111100,1100000011,1100000000,1100011111,1100000011,1100000011,0011111100",
    ['H'] = "1100000011,1100000011,1100000011,1111111111,1100000011,1100000011,1100000011",
    ['I'] = "0011111100,0000110000,0000110000,0000110000,0000110000,0000110000,0011111100",
    ['J'] = "0000111111,0000001100,0000001100,0000001100,1100001100,1100001100,0011110000",
    ['K'] = "1100000011,1100001100,1100110000,1111000000,1100110000,1100001100,1100000011",
    ['L'] = "1100000000,1100000000,1100000000,1100000000,1100000000,1100000000,1111111111",
    ['M'] = "1100000011,1111001111,1100110011,1100110011,1100000011,1100000011,1100000011",
    ['N'] = "1100000011,1111000011,1100110011,1100001111,1100000011,1100000011,1100000011",
    ['O'] = "0011111100,1100000011,1100000011,1100000011,1100000011,1100000011,0011111100",
    ['P'] = "1111111100,1100000011,1100000011,1111111100,1100000000,1100000000,1100000000",
    ['Q'] = "0011111100,1100000011,1100000011,1100000011,1100110011,1100001100,0011110011",
    ['R'] = "1111111100,1100000011,1100000011,1111111100,1100110000,1100001100,1100000011",
    ['S'] = "0011111100,1100000011,1100000000,0011111100,0000000011,1100000011,0011111100",
    ['T'] = "1111111111,0000110000,0000110000,0000110000,0000110000,0000110000,0000110000",
    ['U'] = "1100000011,1100000011,1100000011,1100000011,1100000011,1100000011,0011111100",
    ['V'] = "1100000011,1100000011,1100000011,1100000011,1100000011,0011001100,0000110000",
    ['W'] = "1100000011,1100000011,1100000011,1100110011,1100110011,1111001111,1100000011",
    ['X'] = "1100000011,1100000011,0011001100,0000110000,0011001100,1100000011,1100000011",
    ['Y'] = "1100000011,1100000011,0011001100,0000110000,0000110000,0000110000,0000110000",
    ['Z'] = "1111111111,0000000011,0000001100,0000110000,0011000000,1100000000,1111111111",
};

/* Cyrillic font patterns (Russian А-Я) */
typedef struct { wchar_t ch; const char *pattern; } cyrillic_font_t;
cyrillic_font_t font_cyrillic[] = {
    {L'А', "0011111100,1100000011,1100000011,1111111111,1100000011,1100000011,1100000011"},
    {L'Б', "1111111111,1100000000,1100000000,1111111100,1100000011,1100000011,1111111100"},
    {L'В', "1111111100,1100000011,1100000011,1111111100,1100000011,1100000011,1111111100"},
    {L'Г', "1111111111,1100000000,1100000000,1100000000,1100000000,1100000000,1100000000"},
    {L'Д', "0000111100,0011001100,0011001100,0011001100,0011001100,1111111111,1100000011"},
    {L'Е', "1111111111,1100000000,1100000000,1111111100,1100000000,1100000000,1111111111"},
    {L'Ё', "1100000011,1111111111,1100000000,1111111100,1100000000,1100000000,1111111111"},
    {L'Ж', "1100110011,1100110011,0011111100,0000110000,0011111100,1100110011,1100110011"},
    {L'З', "0011111100,1100000011,0000000011,0000111100,0000000011,1100000011,0011111100"},
    {L'И', "1100000011,1100000011,1100000111,1100001111,1100110011,1111000011,1100000011"},
    {L'Й', "0011111100,1100000011,1100000111,1100001111,1100110011,1111000011,1100000011"},
    {L'К', "1100000011,1100001100,1100110000,1111000000,1100110000,1100001100,1100000011"},
    {L'Л', "0000111111,0011000011,0011000011,0011000011,0011000011,0011000011,1100000011"},
    {L'М', "1100000011,1111001111,1100110011,1100110011,1100000011,1100000011,1100000011"},
    {L'Н', "1100000011,1100000011,1100000011,1111111111,1100000011,1100000011,1100000011"},
    {L'О', "0011111100,1100000011,1100000011,1100000011,1100000011,1100000011,0011111100"},
    {L'П', "1111111111,1100000011,1100000011,1100000011,1100000011,1100000011,1100000011"},
    {L'Р', "1111111100,1100000011,1100000011,1111111100,1100000000,1100000000,1100000000"},
    {L'С', "0011111100,1100000011,1100000000,1100000000,1100000000,1100000011,0011111100"},
    {L'Т', "1111111111,0000110000,0000110000,0000110000,0000110000,0000110000,0000110000"},
    {L'У', "1100000011,1100000011,0011001100,0000110000,0000110000,0011000000,1100000000"},
    {L'Ф', "0000110000,0011111100,1100110011,1100110011,1100110011,0011111100,0000110000"},
    {L'Х', "1100000011,1100000011,0011001100,0000110000,0011001100,1100000011,1100000011"},
    {L'Ц', "1100001100,1100001100,1100001100,1100001100,1100001100,1111111111,0000000011"},
    {L'Ч', "1100000011,1100000011,1100000011,0011111111,0000000011,0000000011,0000000011"},
    {L'Ш', "1100110011,1100110011,1100110011,1100110011,1100110011,1100110011,1111111111"},
    {L'Щ', "1100110011,1100110011,1100110011,1100110011,1100110011,1111111111,0000000011"},
    {L'Ъ', "1111000000,0011000000,0011000000,0011111100,0011000011,0011000011,0011111100"},
    {L'Ы', "1100000011,1100000011,1100000011,1111000011,1100110011,1100110011,1111000011"},
    {L'Ь', "1100000000,1100000000,1100000000,1111111100,1100000011,1100000011,1111111100"},
    {L'Э', "0011111100,1100000011,0000000011,0000111111,0000000011,1100000011,0011111100"},
    {L'Ю', "1100011110,1100110011,1100110011,1111110011,1100110011,1100110011,1100011110"},
    {L'Я', "0011111111,1100000011,1100000011,0011111111,0011000011,0110000011,1100000011"},
    {0, NULL}
};

/* Get font pattern for any character */
const char* get_font_pattern(wchar_t ch) {
    /* Check Latin */
    if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
    if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ':' || ch == ' ') {
        return font_latin[(int)ch];
    }
    /* Check Cyrillic */
    if (ch >= L'а' && ch <= L'я') ch = ch - L'а' + L'А';  /* lowercase to uppercase */
    if (ch == L'ё') ch = L'Ё';
    for (int i = 0; font_cyrillic[i].pattern != NULL; i++) {
        if (font_cyrillic[i].ch == ch) return font_cyrillic[i].pattern;
    }
    return font_latin[' '];  /* default: space */
}

/* Waterfall text bitmap */
#define MAX_WF_WIDTH 256
#define MAX_WF_HEIGHT 7
char wf_bitmap[MAX_WF_HEIGHT][MAX_WF_WIDTH];
int wf_width = 0;
int wf_start_x = 0;
int wf_start_y = 0;

void build_waterfall_bitmap(const char *text) {
    /* Convert UTF-8 to wide chars */
    wchar_t wtext[256];
    mbstowcs(wtext, text, 255);
    wtext[255] = 0;
    int len = wcslen(wtext);

    int char_width = 10;  /* Double width for j+=2 stepping */
    int spacing = 2;
    wf_width = len * (char_width + spacing);
    if (wf_width > MAX_WF_WIDTH) wf_width = MAX_WF_WIDTH;

    memset(wf_bitmap, 0, sizeof(wf_bitmap));

    int x_offset = 0;
    for (int c = 0; c < len && x_offset < MAX_WF_WIDTH; c++) {
        wchar_t ch = wtext[c];
        const char *pattern = get_font_pattern(ch);
        if (!pattern) continue;

        /* Parse pattern "xxxxxxxxxx,xxxxxxxxxx,..." */
        int row = 0;
        int col = 0;
        for (int i = 0; pattern[i] && row < 7; i++) {
            if (pattern[i] == ',') {
                row++;
                col = 0;
            } else {
                if (x_offset + col < MAX_WF_WIDTH) {
                    wf_bitmap[row][x_offset + col] = (pattern[i] == '1') ? 1 : 0;
                }
                col++;
            }
        }
        x_offset += char_width + spacing;
    }
}

/* Greek + math (single-width only) */
wchar_t math_chars[] = {
    L'α', L'β', L'γ', L'δ', L'ε', L'ζ', L'η', L'θ', L'ι', L'κ',
    L'λ', L'μ', L'ν', L'ξ', L'π', L'ρ', L'σ', L'τ', L'υ', L'φ',
    L'χ', L'ψ', L'ω', L'Γ', L'Δ', L'Θ', L'Λ', L'Ξ', L'Π', L'Σ',
    L'Φ', L'Ψ', L'Ω',
    L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9',
    L'+', L'-', L'=', L'x', L'y', L'z', L'n', L'f'
};
int math_chars_len = sizeof(math_chars) / sizeof(wchar_t);
#define RAND_CHAR (mathmode ? math_chars[rand() % math_chars_len] : (rand() % randnum + randmin))
cmatrix **matrix = (cmatrix **) NULL;
int *length = NULL;  /* Length of cols in each line */
int *spaces = NULL;  /* Spaces left to fill */
int *updates = NULL; /* What does this do again? */
#ifndef _WIN32
volatile sig_atomic_t signal_status = 0; /* Indicates a caught signal */
#endif

int va_system(char *str, ...) {

    va_list ap;
    char buf[133];

    va_start(ap, str);
    vsnprintf(buf, sizeof(buf), str, ap);
    va_end(ap);
    return system(buf);
}

/* What we do when we're all set to exit */
void finish(void) {
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
    if (console) {
#ifdef HAVE_CONSOLECHARS
        va_system("consolechars -d");
#elif defined(HAVE_SETFONT)
        va_system("setfont");
#endif
    }
    exit(0);
}

/* What we do when we're all set to exit */
void c_die(char *msg, ...) {

    va_list ap;

    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();

    if (console) {
#ifdef HAVE_CONSOLECHARS
        va_system("consolechars -d");
#elif defined(HAVE_SETFONT)
        va_system("setfont");
#endif
    }

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    exit(0);
}

void usage(void) {
    printf(" Usage: cmatrix -[abBcfhlsmVxk] [-u delay] [-C color] [-t tty] [-M message] [-W text]\n");
    printf(" -a: Asynchronous scroll\n");
    printf(" -b: Bold characters on\n");
    printf(" -B: All bold characters (overrides -b)\n");
    printf(" -c: Use Japanese characters as seen in the original matrix. Requires appropriate fonts\n");
    printf(" -f: Force the linux $TERM type to be on\n");
    printf(" -l: Linux mode (uses matrix console font)\n");
    printf(" -L: Lock mode (can be closed from another terminal)\n");
    printf(" -o: Use old-style scrolling\n");
    printf(" -h: Print usage and exit\n");
    printf(" -n: No bold characters (overrides -b and -B, default)\n");
    printf(" -s: \"Screensaver\" mode, exits on first keystroke\n");
    printf(" -x: X window mode, use if your xterm is using mtx.pcf\n");
    printf(" -V: Print version information and exit\n");
    printf(" -M [message]: Prints your message in the center of the screen. Overrides -L's default message.\n");
    printf(" -W [text]: Waterfall mode - text appears through the rain (Brno fountain style)\n");
    printf(" -u delay (0 - 10, default 4): Screen update delay\n");
    printf(" -C [color]: Use this color for matrix (default green)\n");
    printf(" -r: rainbow mode\n");
    printf(" -m: lambda mode\n");
    printf(" -k: Characters change while scrolling. (Works without -o opt.)\n");
    printf(" -t [tty]: Set tty to use\n");
}

void version(void) {
    printf(" CMatrix version %s (compiled %s, %s)\n",
        VERSION, __TIME__, __DATE__);
    printf("Email: abishekvashok@gmail.com\n");
    printf("Web: https://github.com/abishekvashok/cmatrix\n");
}


/* nmalloc from nano by Big Gaute */
void *nmalloc(size_t howmuch) {
    void *r;

    if (!(r = malloc(howmuch))) {
        c_die("CMatrix: malloc: out of memory!");
    }

    return r;
}

/* Initialize the global variables */
void var_init() {
    int i, j;

    if (matrix != NULL) {
        free(matrix[0]);
        free(matrix);
    }

    matrix = nmalloc(sizeof(cmatrix *) * (LINES + 1));
    matrix[0] = nmalloc(sizeof(cmatrix) * (LINES + 1) * COLS);
    for (i = 1; i <= LINES; i++) {
        matrix[i] = matrix[i - 1] + COLS;
    }

    if (length != NULL) {
        free(length);
    }
    length = nmalloc(COLS * sizeof(int));

    if (spaces != NULL) {
        free(spaces);
    }
    spaces = nmalloc(COLS* sizeof(int));

    if (updates != NULL) {
        free(updates);
    }
    updates = nmalloc(COLS * sizeof(int));

    /* Make the matrix */
    for (i = 0; i <= LINES; i++) {
        for (j = 0; j <= COLS - 1; j += 2) {
            matrix[i][j].val = -1;
        }
    }

    for (j = 0; j <= COLS - 1; j += 2) {
        /* Set up spaces[] array of how many spaces to skip */
        spaces[j] = (int) rand() % LINES + 1;

        /* And length of the stream */
        length[j] = (int) rand() % (LINES - 3) + 3;

        /* Sentinel value for creation of new objects */
        matrix[1][j].val = ' ';

        /* And set updates[] array for update speed. */
        updates[j] = (int) rand() % 3 + 1;
    }

}

#ifndef _WIN32
void sighandler(int s) {
    signal_status = s;
}
#endif

void resize_screen(void) {
#ifdef _WIN32
    BOOL result;
    HANDLE hStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

    hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdHandle == INVALID_HANDLE_VALUE)
        return;
#else
    char *tty;
    int fd = 0;
    int result = 0;
    struct winsize win;

    tty = ttyname(0);
    if (!tty) {
        return;
#endif
#ifdef _WIN32
    result = GetConsoleScreenBufferInfo(hStdHandle, &csbiInfo);
    if (!result)
        return;
    LINES = csbiInfo.dwSize.Y;
    COLS = csbiInfo.dwSize.X;
#else
    }
    fd = open(tty, O_RDWR);
    if (fd == -1) {
        return;
    }
    result = ioctl(fd, TIOCGWINSZ, &win);
    if (result == -1) {
        return;
    }

    COLS = win.ws_col;
    LINES = win.ws_row;
#endif

    if (LINES < 10) {
        LINES = 10;
    }
    if (COLS < 10) {
        COLS = 10;
    }

#ifdef HAVE_RESIZETERM
    resizeterm(LINES, COLS);
#ifdef HAVE_WRESIZE
    if (wresize(stdscr, LINES, COLS) == ERR) {
        c_die("Cannot resize window!");
    }
#endif /* HAVE_WRESIZE */
#endif /* HAVE_RESIZETERM */

    var_init();
    /* Do these because width may have changed... */
    clear();
    refresh();
}

int main(int argc, char *argv[]) {
    int i, y, z, optchr, keypress;
    int j = 0;
    int count = 0;
    int screensaver = 0;
    int asynch = 0;
    int bold = 0;
    int force = 0;
    int firstcoldone = 0;
    int oldstyle = 0;
    int random = 0;
    int update = 4;
    int highnum = 0;
    int mcolor = COLOR_GREEN;
    int rainbow = 0;
    int lambda = 0;
    int randnum = 0;
    int randmin = 0;
    int pause = 0;
    int classic = 0;
    int changes = 0;
    char *msg = "";
    char *tty = NULL;

    srand((unsigned) time(NULL));
    setlocale(LC_ALL, "");

    /* Many thanks to morph- (morph@jmss.com) for this getopt patch */
    opterr = 0;
    while ((optchr = getopt(argc, argv, "abBcfhlLnrosmxkVM:u:C:t:EW:")) != EOF) {
        switch (optchr) {
        case 's':
            screensaver = 1;
            break;
        case 'a':
            asynch = 1;
            break;
        case 'b':
            if (bold != 2) {
                bold = 1;
            }
            break;
        case 'B':
            bold = 2;
            break;
        case 'C':
            if (!strcasecmp(optarg, "green")) {
                mcolor = COLOR_GREEN;
            } else if (!strcasecmp(optarg, "red")) {
                mcolor = COLOR_RED;
            } else if (!strcasecmp(optarg, "blue")) {
                mcolor = COLOR_BLUE;
            } else if (!strcasecmp(optarg, "white")) {
                mcolor = COLOR_WHITE;
            } else if (!strcasecmp(optarg, "yellow")) {
                mcolor = COLOR_YELLOW;
            } else if (!strcasecmp(optarg, "cyan")) {
                mcolor = COLOR_CYAN;
            } else if (!strcasecmp(optarg, "magenta")) {
                mcolor = COLOR_MAGENTA;
            } else if (!strcasecmp(optarg, "black")) {
                mcolor = COLOR_BLACK;
            } else {
                c_die(" Invalid color selection\n Valid "
                       "colors are green, red, blue, "
                       "white, yellow, cyan, magenta " "and black.\n");
            }
            break;
        case 'c':
            classic = 1;
            break;
        case 'E':
            mathmode = 1;
            break;
        case 'f':
            force = 1;
            break;
        case 'l':
            console = 1;
            break;
        case 'L':
            lock = 1;
            //if -M was used earlier, don't override it
            if (0 == strncmp(msg, "", 1)) {
                msg = "Computer locked.";
            }
            break;
        case 'M':
            msg = strdup(optarg);
            break;
        case 'W':
            waterfall = 1;
            waterfall_text = strdup(optarg);
            break;
        case 'n':
            bold = -1;
            break;
        case 'h':
        case '?':
            usage();
            exit(0);
        case 'o':
            oldstyle = 1;
            break;
        case 'u':
            update = atoi(optarg);
            break;
        case 'x':
            xwindow = 1;
            break;
        case 'V':
            version();
            exit(0);
        case 'r':
            rainbow = 1;
            break;
        case 'm':
            lambda = 1;
            break;
        case 'k':
            changes = 1;
            break;
        case 't':
            tty = optarg;
            break;
        }
    }

    /* Clear TERM variable on Windows */
#ifdef _WIN32
    _putenv_s("TERM", "");
#endif

    if (force && strcmp("linux", getenv("TERM"))) {
#ifdef _WIN32
        SetEnvironmentVariableW(L"TERM", L"linux");
#else
        /* setenv is much more safe to use than putenv */
        setenv("TERM", "linux", 1);
#endif
    }
    if (tty) {
        FILE *ftty = fopen(tty, "r+");
        if (!ftty) {
            fprintf(stderr, "cmatrix: error: '%s' couldn't be opened: %s.\n",
                    tty, strerror(errno));
            exit(EXIT_FAILURE);
        }
        SCREEN *ttyscr;
        ttyscr = newterm(NULL, ftty, ftty);
        if (ttyscr == NULL)
            exit(EXIT_FAILURE);
        set_term(ttyscr);
    } else
        initscr();
    savetty();
    nonl();
#ifdef _WIN32
    raw();
#else
    cbreak();
#endif
    noecho();
    timeout(0);
    leaveok(stdscr, TRUE);
    curs_set(0);
#ifndef _WIN32
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGWINCH, sighandler);
    signal(SIGTSTP, sighandler);
#endif

if (console) {
#ifdef HAVE_CONSOLECHARS
        if (va_system("consolechars -f matrix") != 0) {
            c_die
                (" There was an error running consolechars. Please make sure the\n"
                 " consolechars program is in your $PATH.  Try running \"consolechars -f matrix\" by hand.\n");
        }
#elif defined(HAVE_SETFONT)
        if (va_system("setfont matrix") != 0) {
            c_die
                (" There was an error running setfont. Please make sure the\n"
                 " setfont program is in your $PATH.  Try running \"setfont matrix\" by hand.\n");
        }
#else
        c_die(" Unable to use both \"setfont\" and \"consolechars\".\n");
#endif
}
    if (has_colors()) {
        start_color();
        /* Add in colors, if available */
#ifdef HAVE_USE_DEFAULT_COLORS
        if (use_default_colors() != ERR) {
            init_pair(COLOR_BLACK, -1, -1);
            init_pair(COLOR_GREEN, COLOR_GREEN, -1);
            init_pair(COLOR_WHITE, COLOR_WHITE, -1);
            init_pair(COLOR_RED, COLOR_RED, -1);
            init_pair(COLOR_CYAN, COLOR_CYAN, -1);
            init_pair(COLOR_MAGENTA, COLOR_MAGENTA, -1);
            init_pair(COLOR_BLUE, COLOR_BLUE, -1);
            init_pair(COLOR_YELLOW, COLOR_YELLOW, -1);
        } else {
#else
        { /* Hack to deal the after effects of else in HAVE_USE_DEFAULT_COLOURS */
#endif
            init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
            init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        }
    }

    /* Set up values for random number generation */
    if (classic) {
        /* Half-width kana characters. In the movie they are y-axis flipped, and
         * they appear alongside latin characters and numerals, but this is the
         * closest we can do with a standard unicode set and a single number
         * range */
        randmin = 0xff66;
        highnum = 0xff9d;
    } else if (console || xwindow) {
        randmin = 166;
        highnum = 217;
    } else {
        randmin = 33;
        highnum = 123;
    }
    randnum = highnum - randmin;

    var_init();

    /* Initialize waterfall text bitmap */
    if (waterfall && waterfall_text) {
        build_waterfall_bitmap(waterfall_text);
        wf_start_y = ((COLS - wf_width) / 2) & ~1;  /* Align to even column */
        wf_start_x = (LINES - MAX_WF_HEIGHT) / 2;
    }

    while (1) {
#ifndef _WIN32
        /* Check for signals */
        if (signal_status == SIGINT || signal_status == SIGQUIT) {
            if (lock != 1)
                finish();
            /* exits */
        }
        if (signal_status == SIGWINCH) {
            resize_screen();
            signal_status = 0;
        }

        if (signal_status == SIGTSTP) {
            if (lock != 1)
                    finish();
        }
#endif

        count++;
        if (count > 4) {
            count = 1;
        }

        if ((keypress = wgetch(stdscr)) != ERR) {
            if (screensaver == 1) {
#ifdef USE_TIOCSTI
                char *str = malloc(0);
                size_t str_len = 0;
                do {
                    str = realloc(str, str_len + 1);
                    str[str_len++] = keypress;
                } while ((keypress = wgetch(stdscr)) != ERR);
                size_t i;
                for (i = 0; i < str_len; i++)
                    ioctl(STDIN_FILENO, TIOCSTI, (char*)(str + i));
                free(str);
#endif
                finish();
            } else {
                switch (keypress) {
#ifdef _WIN32
                case 3: /* Ctrl-C. Fall through */
#endif
                case 'q':
                    if (lock != 1)
                        finish();
                    break;
                case 'a':
                    asynch = 1 - asynch;
                    break;
                case 'b':
                    bold = 1;
                    break;
                case 'B':
                    bold = 2;
                    break;
                case 'L':
                    lock = 1;
                    break;
                case 'n':
                    bold = 0;
                    break;
                case '0': /* Fall through */
                case '1': /* Fall through */
                case '2': /* Fall through */
                case '3': /* Fall through */
                case '4': /* Fall through */
                case '5': /* Fall through */
                case '6': /* Fall through */
                case '7': /* Fall through */
                case '8': /* Fall through */
                case '9':
                    update = keypress - 48;
                    break;
                case '!':
                    mcolor = COLOR_RED;
                    rainbow = 0;
                    break;
                case '@':
                    mcolor = COLOR_GREEN;
                    rainbow = 0;
                    break;
                case '#':
                    mcolor = COLOR_YELLOW;
                    rainbow = 0;
                    break;
                case '$':
                    mcolor = COLOR_BLUE;
                    rainbow = 0;
                    break;
                case '%':
                    mcolor = COLOR_MAGENTA;
                    rainbow = 0;
                    break;
                case 'r':
                     rainbow = 1;
                     break;
                case 'm':
                     lambda = !lambda;
                     break;
                case '^':
                    mcolor = COLOR_CYAN;
                    rainbow = 0;
                    break;
                case '&':
                    mcolor = COLOR_WHITE;
                    rainbow = 0;
                    break;
                case 'p':
                case 'P':
                    pause = (pause == 0)?1:0;
                    break;

                }
            }
        }
        for (j = 0; j <= COLS - 1; j += 2) {
            if ((count > updates[j] || asynch == 0) && pause == 0) {

                /* I don't like old-style scrolling, yuck */
                if (oldstyle) {
                    for (i = LINES - 1; i >= 1; i--) {
                        matrix[i][j].val = matrix[i - 1][j].val;
                    }
                    random = (int) rand() % (randnum + 8) + randmin;

                    if (matrix[1][j].val == 0) {
                        matrix[0][j].val = 1;
                    } else if (matrix[1][j].val == ' '
                             || matrix[1][j].val == -1) {
                        if (spaces[j] > 0) {
                            matrix[0][j].val = ' ';
                            spaces[j]--;
                        } else {

                            /* Random number to determine whether head of next column
                               of chars has a white 'head' on it. */

                            if (((int) rand() % 3) == 1) {
                                matrix[0][j].val = 0;
                            } else {
                                matrix[0][j].val = RAND_CHAR;
                            }
                            spaces[j] = (int) rand() % LINES + 1;
                        }
                    } else if (random > highnum && matrix[1][j].val != 1) {
                        matrix[0][j].val = ' ';
                    } else {
                        matrix[0][j].val = RAND_CHAR;
                    }

                } else { /* New style scrolling (default) */
                    if (matrix[0][j].val == -1 && matrix[1][j].val == ' '
                        && spaces[j] > 0) {
                        spaces[j]--;
                    } else if (matrix[0][j].val == -1
                        && matrix[1][j].val == ' ') {
                        length[j] = (int) rand() % (LINES - 3) + 3;
                        matrix[0][j].val = RAND_CHAR;

                        spaces[j] = (int) rand() % LINES + 1;
                    }
                    i = 0;
                    y = 0;
                    firstcoldone = 0;
                    while (i <= LINES) {

                        /* Skip over spaces */
                        while (i <= LINES && (matrix[i][j].val == ' ' ||
                               matrix[i][j].val == -1)) {
                            i++;
                        }

                        if (i > LINES) {
                            break;
                        }

                        /* Go to the head of this column */
                        z = i;
                        y = 0;
                        while (i <= LINES && (matrix[i][j].val != ' ' &&
                               matrix[i][j].val != -1)) {
                            matrix[i][j].is_head = false;
                            if (changes) {
                                if (rand() % 8 == 0)
                                    matrix[i][j].val = RAND_CHAR;
                            }
                            i++;
                            y++;
                        }

                        if (i > LINES) {
                            matrix[z][j].val = ' ';
                            continue;
                        }

                        matrix[i][j].val = RAND_CHAR;
                        matrix[i][j].is_head = true;

                        /* If we're at the top of the column and it's reached its
                           full length (about to start moving down), we do this
                           to get it moving.  This is also how we keep segments not
                           already growing from growing accidentally =>
                         */
                        if (y > length[j] || firstcoldone) {
                            matrix[z][j].val = ' ';
                            matrix[0][j].val = -1;
                        }
                        firstcoldone = 1;
                        i++;
                    }
                }
            }
            /* A simple hack */
            if (!oldstyle) {
                y = 1;
                z = LINES;
            } else {
                y = 0;
                z = LINES - 1;
            }
            for (i = y; i <= z; i++) {
                move(i - y, j);

                /* Check if this position is a waterfall text pixel */
                int is_wf_pixel = 0;
                if (waterfall) {
                    int screen_row = i - y;
                    int screen_col = j;
                    int wf_row = screen_row - wf_start_x;
                    int wf_col = screen_col - wf_start_y;
                    if (wf_row >= 0 && wf_row < MAX_WF_HEIGHT &&
                        wf_col >= 0 && wf_col < wf_width) {
                        is_wf_pixel = wf_bitmap[wf_row][wf_col];
                    }
                }

                /* Waterfall pixel - DARK MASK mode */
                /* Text appears as dark/empty areas carved into the rain */
                if (is_wf_pixel) {
                    /* 90% chance to be dark (empty), 10% chance dim flicker */
                    if (rand() % 100 < 90) {
                        addch(' ');  /* Dark - no character */
                    } else {
                        /* Rare dim flicker to make it feel alive */
                        attron(COLOR_PAIR(COLOR_BLACK));
                        wchar_t char_array[2];
                        char_array[0] = rand() % randnum + randmin;
                        char_array[1] = 0;
                        addwstr(char_array);
                        attroff(COLOR_PAIR(COLOR_BLACK));
                    }
                } else if (matrix[i][j].val == 0 || (matrix[i][j].is_head && !rainbow)) {
                    if (console || xwindow) {
                        attron(A_ALTCHARSET);
                    }
                    attron(COLOR_PAIR(COLOR_WHITE));
                    if (bold) {
                        attron(A_BOLD);
                    }
                    if (matrix[i][j].val == 0) {
                        if (console || xwindow) {
                            addch(183);
                        } else {
                            addch('&');
                        }
                    } else if (matrix[i][j].val == -1) {
                        addch(' ');
                    } else {
                        addch(matrix[i][j].val);
                    }

                    attroff(COLOR_PAIR(COLOR_WHITE));
                    if (bold) {
                        attroff(A_BOLD);
                    }
                    if (console || xwindow) {
                        attroff(A_ALTCHARSET);
                    }
                } else {
                    if (rainbow) {
                        int randomColor = rand() % 6;

                        switch (randomColor) {
                            case 0:
                                mcolor = COLOR_GREEN;
                                break;
                            case 1:
                                mcolor = COLOR_BLUE;
                                break;
                            case 2:
                                mcolor = COLOR_BLACK;
                                break;
                            case 3:
                                mcolor = COLOR_YELLOW;
                                break;
                            case 4:
                                mcolor = COLOR_CYAN;
                                break;
                            case 5:
                                mcolor = COLOR_MAGENTA;
                                break;
                       }
                    }
                    attron(COLOR_PAIR(mcolor));
                    if (matrix[i][j].val == 1) {
                        if (bold) {
                            attron(A_BOLD);
                        }
                        addch('|');
                        if (bold) {
                            attroff(A_BOLD);
                        }
                    } else {
                        if (console || xwindow) {
                            attron(A_ALTCHARSET);
                        }
                        if (bold == 2 ||
                            (bold == 1 && matrix[i][j].val % 2 == 0)) {
                            attron(A_BOLD);
                        }
                        if (matrix[i][j].val == -1) {
                            addch(' ');
                        } else if (lambda && matrix[i][j].val != ' ') {
                            addstr("λ");
                        } else {
                            /* addch doesn't seem to work with unicode
                             * characters and there was no direct equivalent.
                             * So, construct a c-style string with the character
                             * and print that.
                             */
                            wchar_t char_array[2];
                            char_array[0] = matrix[i][j].val;
                            char_array[1] = 0;
                            addwstr(char_array);
                        }
                        if (bold == 2 ||
                            (bold == 1 && matrix[i][j].val % 2 == 0)) {
                            attroff(A_BOLD);
                        }
                        if (console || xwindow) {
                            attroff(A_ALTCHARSET);
                        }
                    }
                    attroff(COLOR_PAIR(mcolor));
                }
            }
        }

        //check if -M and/or -L was used
        if (msg[0] != '\0') {
            //Add our message to the screen
            int msg_x = LINES/2;
            int msg_y = COLS/2 - strlen(msg)/2;
            int i = 0;

            //Add space before message
            move(msg_x-1, msg_y-2);
            for (i = 0; i < strlen(msg)+4; i++)
                addch(' ');

            //Write message
            move(msg_x, msg_y-2);
            addch(' ');
            addch(' ');
            addstr(msg);
            addch(' ');
            addch(' ');

            //Add space after message
            move(msg_x+1, msg_y-2);
            for (i = 0; i < strlen(msg)+4; i++)
                addch(' ');
        }

        napms(update * 10);
    }
    finish();
}
