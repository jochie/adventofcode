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

int *star_totals;
int *star_touching;
int max_rows;
int max_cols;

void parse_options(int *argc, char **argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);

/* Check if a position has a star, and if so, track how many values
   are near it, and the product of their values */
int
check_star(char **map, int row, int col, int val)
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
    if (map[row][col] != '*') {
        return false;
    }
    star_totals[row * max_cols + col] *= val;
    star_touching[row * max_cols + col]++;
    return true;
}

/* Determine the complete number, check all around it */
void
check_all_stars(char **map, int row, int col)
{
    int val = 0;
    int len = 0;

    for (int i = col; i < strlen(map[row]); i++) {
        if (!isdigit(map[row][i])) {
            break;
        }
        val = val * 10 + map[row][i] - '0';
        len++;
    }
    for (int j = -1; j <= 1; j++) {
        check_star(map, row + j, col - 1, val);
        check_star(map, row + j, col + len, val);
    }
    for (int i = 0; i < len; i++) {
        check_star(map, row - 1, col + i, val);
        check_star(map, row + 1, col + i, val);
    }
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
    /* add an empty line, for the check_star() test */
    map[rows] = (char *)malloc(1);
    map[rows][0] = '\0';
    max_rows = rows;
    max_cols = cols;
    
    star_totals   = (int *)calloc(rows * cols, sizeof(int));
    star_touching = (int *)calloc(rows * cols, sizeof(int));

    for (int i = 0; i < rows * cols; i++) {
        star_totals[i] = 1;
        star_touching[i] = 0;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    for (int row = 0; row < rows; row++) {
        int indigit = false;
        for (int col = 0; col < cols; col++) {
            if (isdigit(map[row][col])) {
                int val;

                if (!indigit) {
                    indigit = true;
                    check_all_stars((char **)map, row, col);
                }
            } else {
                indigit = false;
            }
        }
    }
    int gear_ratios = 0;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int i = row * cols + col;
            if (star_touching[i] == 2) {
                gear_ratios += star_totals[i];
            }
        }
    }
    printf("Total gear ratios is %d\n", gear_ratios);

    /* For valgrind ;) */
    for (int i = 0; i <= rows; i++) {
        free(map[i]);
    }
    free(map);
    free(star_totals);
    free(star_touching);
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
     %s - Program for AoC 2023 puzzles; Day 3, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 3, part 2.\n\
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
