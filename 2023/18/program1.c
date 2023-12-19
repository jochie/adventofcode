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
# define DAY    18
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


typedef enum direction {
    UP, RIGHT, DOWN, LEFT
} direction;

/* A quick pre-scan of the input file was done to determine this would
   be sufficient */
# define GRID_SIZE 2200
# define GRID_ZERO 1100

typedef enum gridcell {
    EMPTY,
    TRENCH,
    FLOOD
} gridcell;

gridcell grid[GRID_SIZE][GRID_SIZE];

int min_row = 0, min_col = 0, max_row = 0, max_col = 0;


void
init_grid()
{
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = EMPTY;
        }
    }
}


void
mark_grid(int row, int col, gridcell t)
{
    grid[GRID_ZERO + row][GRID_ZERO + col] = t;
}


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


void
travel_direction(int *row, int *col, direction dir, int distance)
{
    int next_row, next_col;

    for (int i = 0; i < distance; i++) {
        next_position(*row, *col, dir, &next_row, &next_col);
        mark_grid(next_row, next_col, TRENCH);
        *row = next_row;
        *col = next_col;
        if (*row < min_row) {
            min_row = *row;
        }
        if (*row > max_row) {
            max_row = *row;
        }
        if (*col < min_col) {
            min_col = *col;
        }
        if (*col > max_col) {
            max_col = *col;
        }
    }
}


void
dump_grid()
{
    char buf[GRID_SIZE + 1];

    for (int row = min_row; row <= max_row; row++) {
        for (int col = min_col; col <= max_col; col++) {
            switch (grid[GRID_ZERO + row][GRID_ZERO + col]) {
            case EMPTY:
                buf[col - min_col] = '.'; break;
            case TRENCH:
                buf[col - min_col] = '#'; break;
            case FLOOD:
                buf[col - min_col] = '+'; break;
            }
        }
        buf[max_col - min_col + 1] = '\0';
        printf("%s\n", buf);
    }
}


void
flood_fill(int row, int col)
{
    if (grid[GRID_ZERO + row][GRID_ZERO + col] != EMPTY) {
        return;
    }
    mark_grid(row, col, FLOOD);
    for (int d = 0; d < sizeof(direction); d++) {
        int next_row, next_col;

        next_position(row, col, d, &next_row, &next_col);
        if (next_row < min_row || next_row > max_row || next_col < min_col || next_col > max_col) {
            continue;
        }
        flood_fill(next_row, next_col);
    }
}


int
calculate_unflooded()
{
    int total = 0;

    for (int row = min_row; row <= max_row; row++) {
        for (int col = min_col; col <= max_col; col++) {
            if (grid[GRID_ZERO + row][GRID_ZERO + col] != FLOOD) {
                total++;
            }
        }
    }
    return total;
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

    int d_totals[4] = { 0, 0, 0, 0 };
    int row = 0, col = 0;

    init_grid();

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
        char dir;
        int distance;
        char hexcode[7];
        sscanf(buf, "%c %d (#%[0-9a-f])", &dir, &distance, hexcode);
        if (opts.debug) {
            printf("Received direction '%c', distance '%d', and hexcode '%s'\n",
                   dir, distance, hexcode);
        }
        switch (dir) {
        case 'U':
            d_totals[UP] += distance;
            travel_direction(&row, &col, UP, distance);
            break;
        case 'R':
            d_totals[RIGHT] += distance;
            travel_direction(&row, &col, RIGHT, distance);
            break;
        case 'D':
            d_totals[DOWN] += distance;
            travel_direction(&row, &col, DOWN, distance);
            break;
        case 'L':
            d_totals[LEFT] += distance;
            travel_direction(&row, &col, LEFT, distance);
            break;
        default:
            printf("What direction is that? %c\n", dir);
            exit(1);
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (opts.debug) {
        printf("Digging in the four directions:\n");
        for (int d = 0; d < sizeof(direction); d++) {
            printf("  Direction %d: %d\n", d, d_totals[d]);
        }
        printf("Row range; [%d,%d]\n", min_row, max_row);
        printf("Col range; [%d,%d]\n", min_col, max_col);
        printf("\n");
    }
    min_row--;
    min_col--;
    max_row++;
    max_col++;
    if (opts.debug) {
        printf("Before flood fill:\n");
        dump_grid();
    }
    flood_fill(min_row, min_col);
    if (opts.debug) {
        printf("After flood fill:\n");
        dump_grid();
    }
    int count = calculate_unflooded();
    printf("Total unflooded area: %d\n", count);
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
