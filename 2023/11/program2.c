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


void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    char map[150][150];
    int max_row = 0;
    int max_col = 0;

    struct coords {
        int row;
        int col;
        int row_e; /* Expanded row */
        int col_e; /* Expanded col */
    } galaxies[500];
    int galaxies_count = 0;

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
        strcpy(map[max_row], buf);
        for (int j = 0; j < max_col; j++) {
            if (buf[j] == '#') {
                galaxies[galaxies_count].row = max_row;
                galaxies[galaxies_count].col = j;
                galaxies[galaxies_count].row_e = max_row;
                galaxies[galaxies_count].col_e = j;
                galaxies_count++;
            }
        }
        max_row++;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (opts.debug) {
        printf("Galaxies found: %d\n", galaxies_count);
    }

    int expansion = 1000000;
    /*
     * Expand vertically
     */
    for (int i = 0; i < max_row; i++) {
        bool empty = true;
        for (int j = 0; j < max_col; j++) {
            if (map[i][j] != '.') {
                empty = false;
                break;
            }
        }
        if (empty) {
            if (opts.debug) {
                printf("Empty row: %d\n", i + 1);
            }
            for (int g = 0; g < galaxies_count; g++) {
                if (galaxies[g].row > i) {
                    galaxies[g].row_e += expansion - 1;
                }
            }
        }
    }
    /*
     * Expand horizontally
     */
    for (int i = 0; i < max_col; i++) {
        bool empty = true;
        for (int j = 0; j < max_col; j++) {
            if (map[j][i] != '.') {
                empty = false;
                break;
            }
        }
        if (empty) {
            if (opts.debug) {
                printf("Empty column: %d\n", i + 1);
            }
            for (int g = 0; g < galaxies_count; g++) {
                if (galaxies[g].col > i) {
                    galaxies[g].col_e += expansion - 1;
                }
            }
        }
    }

    int pairings = 0;
    long sum_distances = 0;
    for (int i = 0; i < galaxies_count - 1; i++) {
        for (int j = i + 1; j < galaxies_count; j++) {
            pairings++;
            long distance =
                abs(galaxies[i].row_e - galaxies[j].row_e) +
                abs(galaxies[i].col_e - galaxies[j].col_e);
            if (opts.debug) {
                printf("Pairing %d: %d and %d -> %ld\n", pairings, i + 1, j + 1, distance);
            }
            sum_distances += distance;
        }
    }
    printf("Sum of all distances: %ld\n", sum_distances);
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
     %s - Program for AoC 2023 puzzles; Day 11, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 11, part 2.\n\
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
