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

# define YEAR 2023
# define DAY    14
# define PART    1

# define STR(x) _STR(x)
# define _STR(x) #x

# define MAX_LEN 1024

struct {
    bool debug   : 1;
    bool dryrun  : 1;
    bool verbose : 1;
} opts;


void process_file(FILE *);
void parse_options(int *argc, char **argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);


/*
 * Main code block which checks the options and either read files (if
 * provided after the options), or whatever is fed to it on stdin,
 * calling process_file() for it.
 */
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


char map[200][200];

void
print_map(int max_row)
{
    for (int i = 0; i < max_row; i++) {
        printf("%s\n", map[i]);
    }
    printf("\n");
}

void
shift_north(int max_row, int max_col)
{
    for (int col = 0; col < max_col; col++) {
        int target = 0;

        if (opts.debug) {
            printf("Column %d:\n", col);
        }
        for (int row = 0; row < max_row; row++) {
            switch (map[row][col]) {
            case '#':
                target = row + 1;
                if (opts.debug) {
                    printf("(%d,%d) Setting next target to %d\n", row, col, target);
                }
                break;
            case 'O':
                if (row == target) {
                    target++;
                } else {
                    if (opts.debug) {
                        printf("Moving O at (%d,%d) to (%d,%d)\n",
                               row, col, target, col);
                    }
                    map[target][col] = 'O';
                    map[row][col] = '.';
                    target++;
                }
                break;
            case '.':
                break;
            }
        }
    }
}

int
calculate_load(int max_row, int max_col)
{
    int load = 0;

    for (int col = 0; col < max_col; col++) {
        for (int row = 0; row < max_row; row++) {
            if (map[row][col] == 'O') {
                load += (max_row - row);
            }
        }
    }
    return load;
}

/*
 * Read from the filedescriptor (whether it's stdin or an actual file)
 * until we reach the end. Strip newlines, and then do what needs to
 * be done.
 */
void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];
    int max_row = 0, max_col = 0;

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
    if (opts.debug) {
        print_map(max_row);
    }
    shift_north(max_row, max_col);
    if (opts.debug) {
        print_map(max_row);
    }

    int load = calculate_load(max_row, max_col);
    printf("Total load: %d\n", load);
}


/*
 * Minimal option parsing. As it stands, the program actually only
 * does something for -d and -h, but it's the thought that counts?
 */
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


/*
 * Print a minimal usage blurb for the program.
 */
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
     %s - Program for AoC " STR(YEAR) " puzzles; Day " STR(DAY) ", part " STR(PART) "\n \
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
     This program is used for one of the AoC " STR(YEAR) " puzzles; Day " STR(DAY) ", part " STR(PART) ".\n\
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
