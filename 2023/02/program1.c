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

bool
is_possible(char *info, int len)
{
    int red   = 12;
    int green = 13;
    int blue  = 14;

    if (opts.debug) {
        printf("Starting remaining bit to check: %.*s (%d)\n", len, info, len);
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
            if (total > blue) {
                return false;
            }
        } else if (!strcmp(color, "red")) {
            if (opts.debug) {
                printf("  Red: %d\n", total);
            }
            if (total > red) {
                return false;
            }
        } else if (!strcmp(color, "green")) {
            if (opts.debug) {
                printf("  Green: %d\n", total);
            }
            if (total > green) {
                return false;
            }
        }
        if (NULL == (ix = strchr(info, ','))) {
            break;
        }
        /* Skip over the ', ' */
        info = ix + 2;
        len -= (int)(ix - info) + 2;
    }
    return true;
}

int
main(int argc, char *argv[], char *env[])
{
    int sum_ids = 0;
    char buf[MAX_LEN + 1];

    parse_options(argc, argv);

    while (NULL != fgets(buf, MAX_LEN, stdin)) {
        char *info;
        int game_id;
        int game_possible = true;

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

        /* It's possible, until it isn't */
        while (strlen(info)) {
            char *ix, snippet[MAX_LEN + 1];

            if (NULL == (ix = strchr(info, ';'))) {
                if (opts.debug) {
                    printf("  Snippet: %s\n", info);
                }
                game_possible = is_possible(info, strlen(info));
                break;
            }
            if (opts.debug) {
                printf("  Snippet length: %d\n", (int)(ix - info));
                printf("  Snippet: %.*s\n", (int)(ix - info), info);
            }

            strlcpy(snippet, info, 1 + (int)(ix - info));
            game_possible = is_possible(snippet, strlen(snippet));
            if (!game_possible) {
                break;
            }
            /* Step over the '; ' */
            info = ix + 2;
        }
        if (game_possible) {
            if (opts.debug) {
                printf("Game %d is possible.\n", game_id);
            }
            sum_ids += game_id;
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Sum of the IDs of possible games is %d\n", sum_ids);
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
     %s - Program for AoC 2023 puzzles; Day 2, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 2, part 1.\n\
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
