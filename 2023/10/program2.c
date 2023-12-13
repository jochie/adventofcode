/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- for emacs
 *
 * =======================================================================
 * C code template for AoC programs
 * =======================================================================
 */

# include <libgen.h>     /* basename()           */
# include <stdbool.h>    /* bool, true, false    */
# include <stdio.h>      /* FILE, *printf(), etc */
# include <stdlib.h>     /* exit()               */
# include <string.h>     /* strlen()             */
# include <unistd.h>     /* getopt()             */
# include <sys/stat.h>   /* stat()               */
# include <sys/errno.h>  /* errno                */

# define MAX_LEN 1024

struct {
    bool debug;
    bool dryrun;
    bool verbose;
} opts;


void process_file(FILE *);
void parse_options(int *argc, char **argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);


int
main(int argc, char *argv[], char *env[])
{
    parse_options(&argc, &argv);

    if (argc == 0) {
        if (opts.debug) {
            printf("Processing data from stdin.\n");
        }
        process_file(stdin);
        return 0;
    }
    for (int i = 0; i < argc; i++) {
        struct stat st;
        FILE *fd;

        if (stat(argv[i], &st)) {
            printf("File not found: '%s'\n", argv[i]);
            return 1;
        }

        if (opts.debug) {
            printf("Opening %s for reading.\n", argv[i]);
        }
        fd = fopen(argv[i], "r");
        if (fd == NULL) {
            printf("Failed to open '%s': %d (%s)\n", argv[i], errno, strerror(errno));
            return 1;
        }
        if (opts.debug) {
            printf("Processing data from '%s'.\n", argv[i]);
        }
        process_file(fd);
        fclose(fd);
    }
}

char pipes_copy[150][150];
char pipes[150][150];
char plotter[300][300];

void
flood_fill(char c, int row, int col, int max_row, int max_col)
{
    /*    printf("Filling (%d, %d) with %c\n", row, col, c); */
    plotter[row][col] = c;
    if (row > 0 && plotter[row - 1][col] == '.') {
        flood_fill(c, row - 1, col, max_row, max_col);
    }
    if (col > 0 && plotter[row][col - 1] == '.') {
        flood_fill(c, row, col - 1, max_row, max_col);
    }
    if (row < max_row - 1 && plotter[row + 1][col] == '.') {
        flood_fill(c, row + 1, col, max_row, max_col);
    }
    if (col < max_col - 1 && plotter[row][col + 1] == '.') {
        flood_fill(c, row, col + 1, max_row, max_col);
    }
}


enum direction {
    UP    = 0,
    RIGHT,
    DOWN,
    LEFT
};

/*
 * | vertical pipe
 * - horizontal pipe
 * L 90-degree bend
 * J 90-degree bend
 * 7 90-degree bend
 * F 90-degree bend
 * . ground
 * S starting position
 */
void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    int row = -1, col = -1, max_row = 0, max_col = 0;

    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        if (!max_col) {
            max_col = strlen(buf);
        }
        strcpy(pipes[max_row], buf);
        strcpy(pipes_copy[max_row], buf);
        char *pos = strchr(buf, 'S');
        if (NULL != pos) {
            row = max_row;
            col = pos - buf;
            if (opts.debug) {
                printf("Found the starting point at (%d, %d)\n", row, col);
            }
        }
        max_row++;
    }
    int max_p_row = max_row * 2 + 1;
    int max_p_col = max_col * 2 + 1;
    for (int i = 0; i < max_p_row; i++) {
        for (int j = 0; j < max_p_col; j++) {
            plotter[i][j] = '.';
        }
        plotter[i][max_p_col + 1] = '\0';
    }
    if (row == -1 && col == -1) {
        printf("No starting point detected.\n");
        exit(1);
    }
    plotter[row * 2 + 1][col * 2 + 1] = '+';
    /*
     * Just because, figure out what pipe the S represents
     */
    int left  = false;
    int right = false;
    int up    = false;
    int down  = false;
    if (col > 0) {
        switch (pipes[row][col - 1]) {
        case '-': case 'F': case 'L':
            left = true;
            break;
        }
    }
    if (row > 0) {
        switch (pipes[row - 1][col]) {
        case '|': case '7': case 'F':
            up = true;
            break;
        }
    }
    if (col < strlen(pipes[row]) - 1) {
        switch (pipes[row][col + 1]) {
        case '-': case 'J': case '7':
            right = true;
            break;
        }
    }
    if (row < max_row - 1) {
        switch (pipes[row + 1][col]) {
        case '|': case 'L': case 'J':
            down = true;
            break;
        }
    }
    char repl = '.';
    if (left && right) {
        repl = '-';
    } else if (up && down) {
        repl = '|';
    } else if (up && right) {
        repl = 'L';
    } else if (up && left) {
        repl = 'J';
    } else if (down && left) {
        repl = '7';
    } else if (down && right) {
        repl = 'F';
    } else {
        printf("Unexpected combination: (left, %d, up %d, right %d, down %d)\n",
               left, up, right, down);
        exit(1);
    }
    if (opts.debug) {
        printf("Replacement symbol: %c\n", repl);
    }
    pipes[row][col] = repl;
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    enum direction dir; /* up 0, right 1, down 2, left 3 */
    switch (repl) {
    case '|': case 'L': case 'J':
        dir = UP; break;
    case '-': case 'F':
        dir = RIGHT; break;
    case '7':
        dir = DOWN; break;
    }
    int distance = 0;
    while (true) {
        distance++;
        int next_row, next_col;
        pipes[row][col] = '*'; /* Mark as visited */
        plotter[row * 2 + 1][col * 2 + 1] = '+';
        switch (dir) {
        case UP:
            next_row = row - 1;
            next_col = col;
            plotter[row * 2 + 1 - 1][col * 2 + 1] = '+';
            break;
        case RIGHT:
            next_row = row;
            next_col = col + 1;
            plotter[row * 2 + 1][col * 2 + 1 + 1] = '+';
            break;
        case DOWN:
            next_row = row + 1;
            next_col = col;
            plotter[row * 2 + 1 + 1][col * 2 + 1] = '+';
            break;
        case LEFT:
            next_row = row;
            next_col = col - 1;
            plotter[row * 2 + 1][col * 2 + 1 - 1] = '+';
            break;
        }
        char next_symbol = pipes[next_row][next_col];
        if (next_symbol == '*') {
            if (opts.debug) {
                printf("Reached a previously visited location after %d steps.\n", distance);
            }
            break;
        }
        switch (next_symbol) {
        case '|':
        case '-':
            /* Can't change direction */
            break;
        case 'L':
            if (dir == DOWN) {
                /* From down to right */
                dir = RIGHT;
            } else if (dir == LEFT) {
                /* From left to up */
                dir = UP;
            } else {
                printf("Impossible direction change, running into %c at (%d, %d) while going %d\n",
                       next_symbol, next_row, next_col, dir);
                exit(1);
            }
            break;
        case 'J':
            if (dir == DOWN) {
                /* From down to left */
                dir = LEFT;
            } else if (dir == RIGHT) {
                /* From right to up */
                dir = UP;
            } else {
                printf("Impossible direction change, running into %c at (%d, %d) while going %d\n",
                       next_symbol, next_row, next_col, dir);
                exit(1);
            }
            break;
        case '7':
            if (dir == UP) {
                /* From up to left */
                dir = LEFT;
            } else if (dir == RIGHT) {
                /* From right to down */
                dir = DOWN;
            } else {
                printf("Impossible direction change, running into %c at (%d, %d) while going %d\n",
                       next_symbol, next_row, next_col, dir);
                exit(1);
            }
            break;
        case 'F':
            if (dir == UP) {
                /* from up to right */
                dir = RIGHT;
            } else if (dir == LEFT) {
                /* from left to down */
                dir = DOWN;
            } else {
                printf("Impossible direction change, running into %c at (%d, %d) while going %d\n",
                       next_symbol, next_row, next_col, dir);
                exit(1);
            }
            break;
        case '.':
            printf("We ran into the ground at (%d, %d) after %d steps?\n",
                   next_row, next_col, distance);
            exit(1);
        }
        row = next_row;
        col = next_col;
    }

    if (opts.debug) {
        for (int i = 0; i < max_row; i++) {
            printf("%s\n", pipes[i]);
        }
        printf("\n");
        for (int i = 0; i < max_row * 2 + 1; i++) {
            printf("%s\n", plotter[i]);
        }
        printf("\n");
    }

    flood_fill('*', 0, 0, max_p_row, max_p_col);

    if (opts.debug) {
        for (int i = 0; i < max_row * 2 + 1; i++) {
            printf("%s\n", plotter[i]);
        }
        printf("\n");
    }

    int ground = 0;
    for (int i = 0; i < max_row; i++) {
        for (int j = 0; j < max_col; j++) {
            if (plotter[i * 2 + 1][j * 2 + 1] == '.') {
                pipes[i][j] = '.';
                ground++;
            } else {
                pipes[i][j] = '+';
            }
        }
    }

    if (opts.debug) {
        for (int i = 0; i < max_row; i++) {
            printf("%s\n", pipes[i]);
        }
        printf("\n");
    }
    printf("Ground enclosed: %d\n", ground);
}

void
parse_options(int *argc, char **argv[])
{
    int ch;
    char *argv0;

    argv0 = (*argv)[0];

    opts.debug   = false;
    opts.dryrun  = false;
    opts.verbose = false;

    if (*argc == 1) {
        /* What to do if there are no command line arguments */
        *argc -= 1;
        *argv += 1;
        return;
    }
    while ((ch = getopt(*argc, *argv, "dnvh")) != -1) {
        switch (ch) {
        case 'd':
            opts.debug = true;
            break;
        case 'n':
            opts.dryrun = true;
            break;
        case 'v':
            opts.verbose = true;
            break;
        case 'h':
            print_usage(stdout, (*argv)[0], NULL, true, EXIT_SUCCESS);
        case '?':
        default:
            print_usage(stderr, (*argv)[0], "\n", false, EXIT_FAILURE);
        }
    }
    *argc -= optind;
    *argv += optind;
#if 0
    if (*argc > 0) {
        /* What to do with the non-option parameters? */
        print_usage(stderr, argv0, "Too many parameters provided.\n\n", false, EXIT_FAILURE);
    }
#endif
}


void
print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode)
{
    char *name;

    if (prefix != NULL) {
        fprintf(f, "%s", prefix);
    }
    name = basename(argv0);
    fprintf(f, "\
NAME\n\
     %s - Program for AoC 2023 puzzles; Day 10, part 2\n\
\n\
SYNOPSIS\n\
     %s [OPTIONS] [<filename> ...]\n",
            name, name);
    if (!full) {
        exit(exitcode);
    }
    fprintf(f, "\
\n\
DESCRIPTION\n\
     This program is used for one of the AoC 2023 puzzles; Day 10, part 2.\n\
     If filenames are provided, it will process them, one at a time.\n\
     Otherwise it will process whatever it will read from standard input.\n\
\n\
OPTIONS\n\
     -d\n\
        Enable debugging output.\n\
     -n\n\
        Request dryrun (noop) mode.\n\
     -v\n\
        Enable verbose output.\n\
\n\
EXIT STATUS\n\
     The program exits 0 on success, and 1 when something went amiss\n\
     with its option parsing.\n");
    exit(exitcode);
}
