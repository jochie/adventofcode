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
# define DAY    16
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
    UP = 0, RIGHT, DOWN, LEFT
} direction;

char direction_chars[sizeof(direction)] = {'^', '>', 'v', '<'};

direction trans_backslash[sizeof(direction)] = { LEFT, DOWN, RIGHT, UP };
direction trans_slash[sizeof(direction)] = { RIGHT, UP, LEFT, DOWN };

int max_row = 0, max_col = 0;
char map[120][120];

struct meta {
    bool energized  : 1;
    bool seen[sizeof(direction)];
} metadata[120][120];

int energized_total;

void
meta_init(int max_row, int max_col)
{
    for (int i = 0; i < max_row; i++) {
        for (int j = 0; j < max_col; j++) {
            metadata[i][j].energized   = false;
            for (int k = 0; k < sizeof(direction); k++) {
                metadata[i][j].seen[k] = false;
            }
        }
    }
    energized_total = 0;
}

# define MAX_BEAMS 100

struct beam {
    direction dir;
    int row;
    int col;
    bool done;
} beams[MAX_BEAMS];
int beam_count = 0;


void
beam_init()
{
    beam_count = 0;
}

void
beam_add(int row, int col, direction dir)
{
    if (beam_count == MAX_BEAMS) {
        printf("Failed to add another beam. Increase the max (currently %d)\n", MAX_BEAMS);
        exit(1);
    }
    beams[beam_count].row = row;
    beams[beam_count].col = col;
    beams[beam_count].dir = dir;
    beams[beam_count].done = false;
    beam_count++;
}

void
beam_compact()
{
    int target = 0;
    for (int i = 0; i < beam_count; i++) {
        if (beams[i].done) {
            continue;
        }
        if (i == target) {
            target++;
            continue;
        }
        beams[target].dir = beams[i].dir;
        beams[target].row = beams[i].row;
        beams[target].col = beams[i].col;
        beams[target].done = false;
        target++;
    }
    beam_count = target;
}

void
next_position(int *row, int *col, direction dir)
{
    switch (dir) {
    case UP:    (*row)--; break;
    case RIGHT: (*col)++; break;
    case DOWN:  (*row)++; break;
    case LEFT:  (*col)--; break;
    }
}

int
calc_energized_total(int row, int col, direction dir)
{
    /*
     * Now the show begins
     */
    int round = 0;

    meta_init(max_row, max_col);

    beam_init();
    /* Starting position and direction of the first and only beam */
    beam_add(row, col, dir);

    while (true) {
        round++;

        /* Compacting the list of beams */
        beam_compact();
        if (beam_count == 0) {
            break;
        }
        if (opts.debug) {
            printf("Round %d - %d beams, %d squares energized\n",
                   round, beam_count, energized_total);
            for (int i = 0; i < max_row; i++) {
                printf("  %s\n", map[i]);
            }
            printf("\n");
        }
        for (int i = beam_count - 1; i >= 0; i--) {
            if (beams[i].done) {
                continue;
            }
            int cur_row = beams[i].row;
            int cur_col = beams[i].col;
            direction cur_dir = beams[i].dir;

            if (opts.debug) {
                printf("  Beam %d at (%d,%d) - %d\n",
                       i, cur_row, cur_col, cur_dir);
            }
            next_position(&cur_row, &cur_col, cur_dir);
            if (cur_row < 0 || cur_row >= max_row || cur_col < 0 || cur_col >= max_row) {
                if (opts.debug) {
                    printf("  Beam %d left the grid.\n", i);
                }
                beams[i].done = true;
                continue;
            }
            beams[i].row = cur_row;
            beams[i].col = cur_col;

            if (!metadata[cur_row][cur_col].energized) {
                metadata[cur_row][cur_col].energized = true;
                energized_total++;
            }
            if (metadata[cur_row][cur_col].seen[cur_dir]) {
                if (opts.debug) {
                    printf("  Beam %d covers a direction/position seen previously\n", i);
                }
                beams[i].done = true;
                continue;
            }
            metadata[cur_row][cur_col].seen[cur_dir] = true;
            switch (map[cur_row][cur_col]) {
            case '.':
                if (opts.debug) {
                    map[cur_row][cur_col] = direction_chars[cur_dir];
                }
                break;
            case '>': case '<': case 'v': case '^':
                break; /* Originally '.' */
            case '|':
                switch (cur_dir) {
                case LEFT:
                case RIGHT:
                    /* Split into an up and down beam */
                    beams[i].dir = UP;

                    beam_add(cur_row, cur_col, DOWN);
                    break;
                default:
                    break;
                }
                break;
            case '-':
                switch (cur_dir) {
                case UP:
                case DOWN:
                    /* Split into a left and right beam */
                    beams[i].dir = LEFT;

                    beam_add(cur_row, cur_col, RIGHT);
                    break;
                default:
                    break;
                }
                break;
            case '\\':
                beams[i].dir = trans_backslash[cur_dir]; break;
            case '/':
                beams[i].dir = trans_slash[cur_dir]; break;
            default:
                printf("What character did I miss? %c at (%d,%d)\n",
                       map[cur_row][cur_col], cur_row, cur_col);
                exit(1);
            }
        }
    }
    if (opts.debug) {
        printf("Energized spots; %d\n", energized_total);
    }
    return energized_total;
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

    int max_total = 0;
    for (int row = 0; row < max_row; row++) {
        int total = calc_energized_total(row, -1, RIGHT);
        if (opts.debug) {
            printf("Total when starting from (%d,%d): %d\n", row, -1, total);
        }
        if (total > max_total) {
            max_total = total;
        }
        total = calc_energized_total(row, max_col, LEFT);
        if (opts.debug) {
            printf("Total when starting from (%d,%d): %d\n", row, max_col, total);
        }
        if (total > max_total) {
            max_total = total;
        }
    }
    for (int col = 0; col < max_col; col++) {
        int total = calc_energized_total(-1, col, DOWN);
        if (opts.debug) {
            printf("Total when starting from (%d,%d): %d\n", -1, col, total);
        }
        if (total > max_total) {
            max_total = total;
        }
        total = calc_energized_total(max_row, col, UP);
        if (opts.debug) {
            printf("Total when starting from (%d,%d): %d\n", max_row, col, total);
        }
        if (total > max_total) {
            max_total = total;
        }
    }
    printf("Maximum energized spots; %d\n", max_total);
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
