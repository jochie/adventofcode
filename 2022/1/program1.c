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


int
main(int argc, char *argv[], char *env[])
{
    int  elf = 1;
    int  food = 0;
    long calories = 0;
    int  max_elf = 0;
    long max_calories = 0;

    parse_options(argc, argv);

    char buf[MAX_LEN + 1];
    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        long new_calories;

        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        if (strlen(buf) == 0) {
            if (opts.debug) {
                printf("DEBUG: End of food block\n");
            }
            if (food > 0 && calories > 0 && calories > max_calories) {
                max_elf = elf;
                max_calories = calories;
            }

            /* On to the next elf */
            elf++;
            calories = 0;
            food = 0;
        } else {
            int found = 0;

            sscanf(buf, "%d", &found);
            food++;
            calories += found;
            if (opts.debug) {
                printf("DEBUG: Elf %d has accumulated %ld calories with %d item(s).\n", elf, calories, food);
            }
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (food > 0 && calories > 0 && calories > max_calories) {
        max_elf = elf;
        max_calories = calories;
    }
    printf("The elf with the most calories is Elf #%d with %ld calories.\n", max_elf, max_calories);
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
     %s - Program for AoC 2022 puzzles; Day 1, part 1\n\
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
     This program is used for one of the AoC 2022 puzzles; Day 1, part 1.\n\
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
