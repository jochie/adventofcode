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

char map[50][50];

bool
reflecting_rows(int row1, int row2, int max_row, int max_col)
{
    for (int col = 0; col < max_col; col++) {
        if (map[row1][col] != map[row2][col]) {
            return false;
        }
    }
    if (row1 == 0 || row2 == max_row - 1) {
        return true;
    }
    return reflecting_rows(row1 - 1, row2 + 1, max_row, max_col);
}

bool
reflecting_cols(int col1, int col2, int max_row, int max_col)
{
    for (int row = 0; row < max_row; row++) {
        if (map[row][col1] != map[row][col2]) {
            return false;
        }
    }
    if (col1 == 0 || col2 == max_col - 1) {
        return true;
    }
    return reflecting_cols(col1 - 1, col2 + 1, max_row, max_col);
}

int
find_mirror(int max_row, int max_col)
{
    if (opts.debug) {
        printf("Find a mirror in a %d x %d map.\n", max_row, max_col);
    }

    /* Look for a mirror between col and col+1 */
    for (int col = 0; col < max_col - 1; col++) {
        if (reflecting_cols(col, col + 1, max_row, max_col)) {
            if (opts.debug) {
                printf("  Reflecting columns %d and %d\n", col + 1, col + 2);
            }
            return col + 1;
        }
    }

    /* Look for a mirror between row and row+1 */
    for (int row = 0; row < max_row - 1; row++) {
        if (reflecting_rows(row, row + 1, max_row, max_col)) {
            if (opts.debug) {
                printf("  Reflecting between rows %d and %d\n", row + 1, row + 2);
            }
            return (row + 1) * 100;
        }
    }
    return 0;
}

void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];
    int max_row = 0, max_col = 0;
    int summaries = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        if (strlen(buf)) {
            max_col = strlen(buf);
            strcpy(map[max_row], buf);
            max_row++;
        } else {
            if (max_row > 0) {
                summaries += find_mirror(max_row, max_col);
            }
            max_row = 0;
            max_col = 0;
        }
    }
    if (max_row > 0) {
        summaries += find_mirror(max_row, max_col);
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Summarizing all notes: %d\n", summaries);
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
     %s - Program for AoC 2023 puzzles; Day 13, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 13, part 1.\n\
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
