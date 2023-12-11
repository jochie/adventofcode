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

char map[300][300];
char copy[300][300];

int max_row = 0;
int max_col = 0;


void
expand_map()
{
    int new_row = 0;
    for (int i = 0; i < max_row; i++) {
        /* Copy it at least once */
        strcpy(copy[new_row], map[i]);
        new_row++;
        bool empty = true;
        for (int j = 0; j < max_col; j++) {
            if (map[i][j] != '.') {
                empty = false;
                break;
            }
        }
        if (empty) {
            strcpy(copy[new_row], map[i]);
            new_row++;
        }
    }
    max_row = new_row;
    /*
     * Copy it back
     */
    for (int i = 0; i < max_row; i++) {
        strcpy(map[i], copy[i]);
    }
}

void
transpose_map()
{
    for (int i = 0; i < max_col; i++) {
        for (int j = 0; j < max_row; j++) {
            copy[i][j] = map[j][i];
        }
        copy[i][max_row] = '\0';
    }
    int tmp = max_row;
    max_row = max_col;
    max_col = tmp;
    for (int i = 0; i < max_row; i++) {
        strcpy(map[i], copy[i]);
    }
}

void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        strcpy(map[max_row], buf);
        if (!max_col) {
            max_col = strlen(buf);
        }
        max_row++;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    /*
     * Expand the map vertically, transpose it, then expand it again,
     * and finally transpose it one more time to get back to the
     * original orientation
     */
    expand_map();
    transpose_map();
    expand_map();
    transpose_map();

    struct coords {
        int row;
        int col;
    } galaxies[500];
    int galaxies_count = 0;
    for (int i = 0; i < max_row; i++) {
        for (int j = 0; j < max_col; j++) {
            if (map[i][j] == '#') {
                galaxies[galaxies_count].row = i;
                galaxies[galaxies_count].col = j;
                galaxies_count++;
            }
        }
    }
    if (opts.debug) {
        printf("Galaxies found: %d\n", galaxies_count);
    }
    int pairings = 0;
    int sum_distances = 0;
    for (int i = 0; i < galaxies_count - 1; i++) {
        for (int j = i + 1; j < galaxies_count; j++) {
            pairings++;
            int distance =
                abs(galaxies[i].row - galaxies[j].row) +
                abs(galaxies[i].col - galaxies[j].col);
            if (opts.debug) {
                printf("Pairing %d: %d and %d -> %d\n", pairings, i + 1, j + 1, distance);
            }
            sum_distances += distance;
        }
    }
    printf("Sum of all distances: %d\n", sum_distances);
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
     %s - Program for AoC 2023 puzzles; Day 11, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 11, part 1.\n\
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
