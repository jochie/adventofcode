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


void parse_options(int argc, char *argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);


int
main(int argc, char *argv[], char *env[])
{
    parse_options(argc, argv);

    struct translate {
        char *digit;
        int  value;
    } translations[] = {
        { "one",   1 },
        { "two",   2 },
        { "three", 3 },
        { "four",  4 },
        { "five",  5 },
        { "six",   6 },
        { "seven", 7 },
        { "eight", 8 },
        { "nine",  9 }
    };

    char buf[MAX_LEN + 1];
    int sum = 0;
    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        int df = -1;
        int dl = -1;
        for (int i = 0; i < strlen(buf); i++) {
            if (isdigit(buf[i])) {
                int val = buf[i] - '0';
                if (opts.debug) {
                    printf("Digit found: %d\n", val);
                }
                if (df == -1) {
                    df = val;
                }
                dl = val;
            } else {
                for (int j = 0; j < 9; j++) {
                    if (!strncmp(buf + i, translations[j].digit, strlen(translations[j].digit))) {
                        if (opts.debug) {
                            printf("Found a word: %s -> %d\n", translations[j].digit, translations[j].value);
                        }
                        int val = translations[j].value;
                        if (df == -1) {
                            df = val;
                        }
                        dl = val;
                    }
                }
            }
        }
        if (opts.debug) {
            printf("First digit: %d; Last digit: %d\n", df, dl);
        }
        int cal = df * 10 + dl;
        if (opts.debug) {
            printf("Calibration value: %d\n", cal);
        }
        sum += cal;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Sum of the calibration values: %d\n", sum);
}


void
parse_options(int argc, char *argv[])
{
    int ch;

    opts.debug   = false;
    opts.dryrun  = false;
    opts.verbose = false;

    if (argc == 1) {
        /* What to do if there are no command line arguments */
        return;
    }
    while ((ch = getopt(argc, argv, "dnvh")) != -1) {
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
            print_usage(stdout, argv[0], NULL, true, EXIT_SUCCESS);
        case '?':
        default:
            print_usage(stderr, argv[0], "\n", false, EXIT_FAILURE);
        }
    }
    argc -= optind;
    if (argc > 0) {
        /* What to do with the non-option parameters? */
        print_usage(stderr, argv[0], "Too many parameters provided.\n\n", false, EXIT_FAILURE);
    }
    argv += optind;
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
     %s - Program for AoC 2023 puzzles; Day 1, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 1, part 2.\n\
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
