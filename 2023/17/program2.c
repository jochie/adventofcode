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
# define DAY    17
# define PART    2

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

typedef enum direction {
    UP, RIGHT, DOWN, LEFT
} direction;


int max_row = 0, max_col = 0;
char map[150][150];
int min_loss = 0;
int min_dir_loss[150][150][sizeof(direction) * 11];

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

# define MAX_TRACE 10000

void
show_trace(int trace[MAX_TRACE], int count)
{
    char copy[150][150];
    char border[152];

    border[0] = '+';
    border[max_col + 1] = '+';
    for (int col = 0; col < max_col; col++) {
        border[col + 1] = '-';
    }
    border[max_col + 2] = '\0';
    for (int row = 0; row < max_row; row++) {
        strcpy(copy[row], map[row]);
    }
    for (int i = 0; i < count; i++) {
        int t_row = trace[i] / max_col;
        int t_col = trace[i] % max_col;
        copy[t_row][t_col] = ' ';
    }
    copy[max_row - 1][max_col -1] = ' ';
    printf("%s\n", border);
    for (int row = 0; row < max_row; row++) {
        printf("|%s|\n", copy[row]);
    }
    printf("%s\n", border);
}

void
find_loss(int loss, int row, int col, direction dir, int repeats, int trace[MAX_TRACE], int count)
{
    if (opts.debug) {
        printf("find_loss(%d, %d, %d, %d, %d)\n", loss, row, col, dir, repeats);
    }
    if (min_loss > 0 && loss > min_loss) {
        /* No point continuing */
        return;
    }
    /* Try all directions */
    for (int d = 0; d < sizeof(direction); d++) {
        int next_row, next_col;

        if (dir == d + 2 || d == dir + 2) {
            if (opts.debug) {
                printf("Can't reverse (current dir %d, possible dir %d)\n", dir, d);
            }
            continue;
        }
        if (repeats < 4 && dir != d) {
            if (opts.debug) {
                printf("Can't turn yet.\n");
            }
            continue;
        }
        if (dir == d && repeats == 10) {
            if (opts.debug) {
                printf("Can't go another step in that direction (%d)\n", dir);
            }
            continue;
        }
        next_position(row, col, d, &next_row, &next_col);
        if (next_row < 0 || next_col < 0 || next_row == max_row || next_col == max_col) {
            if (opts.debug) {
                printf("That would take us off the board (%d, %d)\n", next_row, next_col);
            }
            continue;
        }
        int new_loss = loss + map[next_row][next_col] - '0';
        if (next_row == max_row - 1 && next_col == max_col - 1) {
            if (min_loss == 0 || new_loss < min_loss) {
                min_loss = new_loss;
                if (opts.verbose) {
                    printf("Reached the destination with loss %d\n", new_loss);
                    if (opts.debug) {
                        printf("TRACE: ");
                        for (int i = 0; i < count; i++) {
                            int t_row = trace[i] / max_col;
                            int t_col = trace[i] % max_col;
                            printf(" (%d,%d)", t_row, t_col);
                        }
                    }
                    show_trace(trace, count);
                    printf("\n");
                }
            }
            continue;
        }
        int dir_loss;
        if (dir == d) {
            dir_loss = repeats * sizeof(direction) + d;
        } else {
            dir_loss = d;
        }
        if (min_dir_loss[next_row][next_col][dir_loss] > 0 && new_loss >= min_dir_loss[next_row][next_col][dir_loss]) {
            if (opts.debug) {
                printf("This is equal or worse than an earlier attempt, for (%d,%d) %d.\n",
                       next_row, next_col, dir_loss);
            }
            continue;
        }
        min_dir_loss[next_row][next_col][dir_loss] = new_loss;
        int new_trace = next_row * max_col + next_col;
        if (count == MAX_TRACE) {
            printf("Oops. Need to increase the trace size.\n");
            exit(1);
        }
        trace[count] = new_trace;
        bool found = false;
        for (int i = 0; i < count; i++ ) {
            if (trace[i] == new_trace) {
                found = true;
            }
        }
        if (found) {
            continue;
        }
        if (dir == d) {
            find_loss(new_loss, next_row, next_col, d, repeats + 1, trace, count + 1);
        } else {
            find_loss(new_loss, next_row, next_col, d, 1, trace, count + 1);
        }
    }
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

    max_row = 0; max_col = 0;
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
    min_loss = 0;
    for (int row = 0; row < max_row; row++) {
        for (int col = 0; col < max_col; col++) {
            for (int i = 0; i < sizeof(direction) * 10; i++) {
                min_dir_loss[row][col][i] = 0;
            }
        }
    }
    int trace[MAX_TRACE];
    trace[0] = 0;
    find_loss(0, 0, 0, RIGHT, 0, trace, 1);
    find_loss(0, 0, 0, DOWN, 0, trace, 1);
    printf("Minimum heat loss found: %d\n", min_loss);
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
