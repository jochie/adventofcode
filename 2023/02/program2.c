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

void
minimum_possible(char *info, int *min_red, int *min_green, int *min_blue)
{
    int len = strlen(info);

    if (opts.debug) {
        printf("Starting remaining bit to check: %s (%d)\n", info, len);
    }

    while (len > 0) {
        int  total;
        char *ix, color[MAX_LEN + 1];

        if (opts.debug) {
            printf("Remaining bit to check: %.*s\n", len, info);
        }
        /* Grab the next number and color, while avoiding to include a
           possible ',' after the color */
        sscanf(info, "%d %[a-z]", &total, color);
        if        (!strcmp(color, "blue")) {
            if (opts.debug) {
                printf("  Blue: %d\n", total);
            }
            if (total > *min_blue) {
                *min_blue = total;
            }
        } else if (!strcmp(color, "red")) {
            if (opts.debug) {
                printf("  Red: %d\n", total);
            }
            if (total > *min_red) {
                *min_red = total;
            }
        } else if (!strcmp(color, "green")) {
            if (opts.debug) {
                printf("  Green: %d\n", total);
            }
            if (total > *min_green) {
                *min_green = total;
            }
        }
        if (NULL == (ix = strchr(info, ','))) {
            break;
        }
        /* Skip over the ', ' */
        info = ix + 2;
        len -= (int)(ix - info) + 2;
    }
}

int
main(int argc, char *argv[], char *env[])
{
    int  sum_powers = 0;
    char buf[MAX_LEN + 1];

    parse_options(argc, argv);

    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        int  game_id;
        char *info;
        int  min_red   = 0;
        int  min_green = 0;
        int  min_blue  = 0;
        int  power;

        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        sscanf(buf, "Game %d", &game_id);
        /* Step over the ': ' */
        info = strchr(buf, ':') + 2;
        if (opts.debug) {
            printf("GAME %d with info '%s'\n", game_id, info);
        }
        while (strlen(info)) {
            char *ix, snippet[MAX_LEN + 1];

            if (NULL == (ix = strchr(info, ';'))) {
                if (opts.debug) {
                    printf("  Snippet: %s\n", info);
                }
                minimum_possible(info, &min_red, &min_green, &min_blue);
                break;
            }
            if (opts.debug) {
                printf("  Snippet length: %d\n", (int)(ix - info));
                printf("  Snippet: %.*s\n", (int)(ix - info), info);
            }

            strlcpy(snippet, info, 1 + (int)(ix - info));
            minimum_possible(snippet, &min_red, &min_green, &min_blue);
            /* Step over the '; ' */
            info = ix + 2;
        }
        power = min_red * min_green * min_blue;
        if (opts.debug) {
            printf("Power of game %d is %d.\n", game_id, power);
        }
        sum_powers += power;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Sum of the powers of the games is %d\n", sum_powers);
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
     %s - Program for AoC 2023 puzzles; Day 2, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 2, part 2.\n\
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
