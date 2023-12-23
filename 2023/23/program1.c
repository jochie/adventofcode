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

# include <openssl/sha.h>

# define YEAR 2023
# define DAY    23
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

/*
 * paths  = .
 * forest = #
 * slopes = ^, >, v, <
 */
char map[200][200];
int max_row, max_col;

bool visited[200][200];

void
visited_init()
{
    for (int row = 0; row < max_row; row++) {
        for (int col = 0; col < max_col; col++) {
            visited[row][col] = false;
        }
    }
}

/* Dealing with directions */
typedef enum direction {
    UP = 0, RIGHT, DOWN, LEFT
} direction;

char slopes[4] = "^>v<";
direction slopes_d[4] = { UP, RIGHT, DOWN, LEFT };

void
next_position(int row, int col, direction dir, int *next_row, int *next_col) {
    *next_row = row;
    *next_col = col;

    switch (dir) {
    case UP:    (*next_row)--; break;
    case RIGHT: (*next_col)++; break;
    case DOWN:  (*next_row)++; break;
    case LEFT:  (*next_col)--; break;
    }
}

int
searching_rec(int row, int col)
{
    if (row == max_row - 1) {
        /* Success */
        return 0;
    }
    char *slope = strchr(slopes, map[row][col]);
    if (NULL != slope) {
        int d = slopes_d[slope - slopes];
        int next_row, next_col;

        next_position(row, col, d, &next_row, &next_col);
        if (visited[next_row][next_col]) {
            return 0;
        }
        visited[next_row][next_col] = true;
        int steps = searching_rec(next_row, next_col);
        visited[next_row][next_col] = false;
        if (steps == -1) {
            /* An unsuccessful journey */
            return -1;
        }
        return 1 + steps;
    }
    int max_steps = -1;
    for (int d = 0; d < sizeof(direction); d++) {
        int next_row, next_col;

        next_position(row, col, d, &next_row, &next_col);
        if (next_row < 0) {
            continue;
        }
        if (map[next_row][next_col] == '#') {
            continue;
        }
        if (visited[next_row][next_col]) {
            continue;
        }
        visited[next_row][next_col] = true;
        int steps = searching_rec(next_row, next_col);
        visited[next_row][next_col] = false;
        if (steps > max_steps) {
            /* Works for -1 for either value as well */
            max_steps = steps;
        }
    }
    if (max_steps == -1) {
        return -1;
    }
    return 1 + max_steps;
}

int
searching()
{
    visited_init();

    visited[0][1] = true;
    int steps = searching_rec(0, 1);
    return steps;
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

    max_row = 0;
    max_col = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            unsigned char digest[SHA256_DIGEST_LENGTH];
            char hexdigest[SHA256_DIGEST_LENGTH * 2 + 1];

            SHA256((unsigned char *)buf, strlen(buf), digest);

            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                sprintf(&hexdigest[i * 2], "%02x", (unsigned int)digest[i]);
            }

            printf("DEBUG: Line received: [%s] '%s'\n", hexdigest, buf);
        }
        if (!max_col) {
            max_col = strlen(buf);
        }
        strcpy(map[max_row], buf);
        max_row++;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    int max_steps = searching();
    printf("Max steps to get there: %d\n", max_steps);
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
