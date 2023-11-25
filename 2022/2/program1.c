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

# define MAX_LEN 1024

struct {
    bool debug;
    bool dryrun;
    bool verbose;
} opts;


void parse_options(int argc, char *argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);

int winning_combination(char, char);
int compare(char, char);
int value(char);


int
main(int argc, char *argv[], char *env[])
{
    char buf[MAX_LEN + 1];
    long score = 0;

    parse_options(argc, argv);

    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        char o_play, i_play, outcome;

        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }

        sscanf(buf, "%c %c", &o_play, &i_play);
        o_play -= 'A';
        i_play -= 'X';
        outcome = compare(o_play, i_play);
        if (opts.debug) {
                printf("They play %d, I play %d; Result = %d\n", o_play, i_play, outcome);
        }
        score += i_play + 1 + outcome * 3;
        if (opts.debug) {
                printf("Score is now %ld\n", score);
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Final score: %ld\n", score);
}

/*
 * A = 0 = rock
 * B = 1 = paper
 * C = 2 = scissors
 *
 * X = 0 = rock
 * Y = 1 = paper
 * Z = 2 = scissors
 */
int
winning_combination(char opp1, char opp2)
{
    if (opp2 == (opp1 + 2) % 3) {
        return true;
    }
    return false;
}


int
compare(char them, char me)
{
    if (them == me) {
        /* Same thing, we draw */
        return 1;
    }
    if (winning_combination(them, me)) {
        /* They win */
        return 0;
    }
    if (winning_combination(me, them)) {
        /* I win */
        return 2;
    }
    fprintf(stderr, "Unexpected combination: They play %d, I play %d?\n", them, me);
    exit(EXIT_FAILURE);
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
     %s - Program for AoC 2022 puzzles; Day 2, part 1\n\
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
     This program is used for one of the AoC 2022 puzzles; Day 2, part 1.\n\
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
