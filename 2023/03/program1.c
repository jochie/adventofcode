/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- for emacs
 *
 * =======================================================================
 * C code template for AoC programs
 * =======================================================================
 */

# include <libgen.h>  /* basename()           */
# include <stdbool.h> /* bool, true, false    */
# include <stdio.h>   /* FILE, *printf(), etc */
# include <stdlib.h>  /* exit()               */
# include <string.h>  /* strlen()             */
# include <unistd.h>  /* getopt()             */
# include <ctype.h>   /* isdigit()            */

# define MAX_LEN 1024

struct {
    bool debug;
    bool dryrun;
    bool verbose;
} opts;


void parse_options(int *argc, char **argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);

/* Are we within the bounds of the map, and do we find a symbol
   here? */
int
check_symbol(char **map, int row, int col)
{
    if (row < 0 || col < 0) {
        return false;
    }
    if (row >= 140) {
        return false;
    }
    if (col >= strlen(map[row])) {
        return false;
    }
    if (isdigit(map[row][col])) {
        return false;
    }
    if (map[row][col] == '.') {
        return false;
    }
    return true;
}

/* Return the value of the number if it touches a symbol, 0 otherwise */
int
touches_symbol(char **map, int row, int col)
{
    int val = 0;
    int len = 0;

    for (int i = col; i < strlen(map[row]); i++) {
        if (isdigit(map[row][i])) {
            val = val * 10 + map[row][i] - '0';
            len++;
        } else {
            break;
        }
    }
    for (int j = -1; j <= 1; j++) {
        if (check_symbol(map, row + j, col - 1)) {
            return val;
        }
        if (check_symbol(map, row + j, col + len)) {
            return val;
        }            
    }
    for (int i = 0; i < len; i++) {
        if (check_symbol(map, row - 1, col + i)) {
            return val;
        }
        if (check_symbol(map, row + 1, col + i)) {
            return val;
        }            
    }
    return 0;
}

int
main(int argc, char *argv[], char *env[])
{
    char **map;
    char buf[MAX_LEN + 1];
    int rows = 0;
    int cols = 0;
    parse_options(&argc, &argv);

    map = (char **)calloc(141, sizeof(char *));
    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        rows++;
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        if (strlen(buf) > cols) {
            cols = strlen(buf);
        }
        map[rows - 1] = (char *)malloc(strlen(buf) + 1);
        strcpy(map[rows - 1], buf);
    }
    /* add an empty line, for the check_symbol() test */
    map[rows] = (char *)malloc(1);
    map[rows][0] = '\0';

    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    int total_val = 0;
    for (int row = 0; row < rows; row++) {
        int indigit = false;
        for (int col = 0; col < cols; col++) {
            if (isdigit(map[row][col])) {
                int val;

                if (!indigit) {
                    indigit = true;
                    if ((val = touches_symbol((char **)map, row, col)) > 0) {
                        total_val += val;
                    }
                }
            } else if (map[row][col] == '.') {
                indigit = false;
                /* Otherwise ignore */
            } else {
                indigit = false;
            }
        }
    }
    printf("Total non-toucing values: %d\n", total_val);

    /* For valgrind ;) */
    for (int i = 0; i <= rows; i++) {
        free(map[i]);
    }
    free(map);
}


void
parse_options(int *argc, char **argv[])
{
    int ch;

    opts.debug   = false;
    opts.dryrun  = false;
    opts.verbose = false;

    if (*argc == 1) {
        /* What to do if there are no command line arguments */
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
    if (*argc > 0) {
        /* What to do with the non-option parameters? */
        print_usage(stderr, (*argv)[0], "Too many parameters provided.\n\n", false, EXIT_FAILURE);
    }
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
     %s - Program for AoC 2023 puzzles; Day 3, part 1\n\
\n\
SYNOPSIS\n\
     %s [OPTIONS]\n",
            name, name);
    if (!full) {
        exit(exitcode);
    }
    fprintf(f, "\
\n\
DESCRIPTION\n\
     This program is used for one of the AoC 2023 puzzles; Day 3, part 1.\n\
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
