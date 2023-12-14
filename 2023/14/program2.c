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
# define PART    2

# define STR(x) _STR(x)
# define _STR(x) #x

# define MAX_LEN 1024

# define MAX_PATTERN 200

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
shift_north(int max_row, int max_col)
{
    for (int col = 0; col < max_col; col++) {
        int target = 0;

        for (int row = 0; row < max_row; row++) {
            switch (map[row][col]) {
            case '#':
                target = row + 1;
                break;
            case 'O':
                if (row == target) {
                    target++;
                } else {
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


void
shift_east(int max_row, int max_col)
{
    for (int row = 0; row < max_row; row++) {
        int target = 0;

        for (int col = 0; col < max_col; col++) {
            switch (map[row][col]) {
            case '#':
                target = col + 1;
                break;
            case 'O':
                if (col == target) {
                    target++;
                } else {
                    map[row][target] = 'O';
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


void
shift_south(int max_row, int max_col)
{
    for (int col = 0; col < max_col; col++) {
        int target = max_row - 1;

        for (int row = max_row - 1; row >= 0; row--) {
            switch (map[row][col]) {
            case '#':
                target = row - 1;
                break;
            case 'O':
                if (row == target) {
                    target--;
                } else {
                    map[target][col] = 'O';
                    map[row][col] = '.';
                    target--;
                }
                break;
            case '.':
                break;
            }
        }
    }
}


void
shift_west(int max_row, int max_col)
{
    for (int row = 0; row < max_row; row++) {
        int target = max_col - 1;

        for (int col = max_col - 1; col >= 0; col--) {
            switch (map[row][col]) {
            case '#':
                target = col - 1;
                break;
            case 'O':
                if (col == target) {
                    target--;
                } else {
                    map[row][target] = 'O';
                    map[row][col] = '.';
                    target--;
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
    int repetitions[MAX_PATTERN];
    for (int i = 0; i < MAX_PATTERN; i++) {
        repetitions[i] = 0;
    }
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
    for (int cycle = 1; cycle < MAX_PATTERN * 5; cycle++) {
        shift_north(max_row, max_col);
        shift_east(max_row, max_col);
        shift_south(max_row, max_col);
        shift_west(max_row, max_col);
        int load = calculate_load(max_row, max_col);
        if (opts.debug) {
            printf("Cycle %d - Total load: %d\n", cycle + 1, load);
        }
        int detections = 0;
        for (int i = 0; i < MAX_PATTERN; i++) {
            if (repetitions[i] == load) {
                detections++;
                if (detections == 2) {
                    int length = i + 1;
                    if (length > 2 && length % 2 == 0) {
                        length /= 2;
                        if (opts.debug) {
                            printf("Found 2 repeats at cycle %d (%d) - checking pattern\n", cycle, i);
                        }
                        bool repeating = true;
                        for (int j = 0; j < length; j++) {
                            if (repetitions[j] != repetitions[j + length]) {
                                repeating = false;
                                break;
                            }
                        }
                        if (repeating) {
                            printf("Confirmed the pattern fully repeats at cycle %d (length of pattern is %d)!\n", cycle, length);
                            long remainder = 1000000000 - cycle;
                            printf("  Remaining cycles: %ld\n", remainder);
                            printf("  Full pattern repeats in that: %ld\n", remainder / length);
                            long cycles_left = remainder % length;
                            printf("  Cycles remaining after that: %ld\n", cycles_left);
                            printf("The final load after those cycles: %d\n",
                                   repetitions[length - 1 - cycles_left]);
                            exit(0);
                        }
                    }
                }
            }
        }
        for (int i = MAX_PATTERN - 1; i >= 1; i--) {
            repetitions[i] = repetitions[i - 1];
        }
        repetitions[0] = load;
    }
    int load = calculate_load(max_row, max_col);
    printf("Final total load: %d\n", load);
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
