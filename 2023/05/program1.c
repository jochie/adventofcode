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

# define MAX_LEN 1024

struct {
    bool debug;
    bool dryrun;
    bool verbose;
} opts;


void process_file(FILE *);
void parse_options(int *argc, char **argv[]);
void print_usage(FILE *f, char *argv0, char *prefix, bool full, int exitcode);


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


void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];
    int state = 0;
    long all_seeds[20];
    long next_stage[20];
    int seeds = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        if (!strlen(buf)) {
            state++;
            for (int i = 0; i < seeds; i++) {
                all_seeds[i] = next_stage[i];
            }
            continue;
        }
        char *info;
        long dest_start, source_start, range_length;
        switch (state) {
        case 0:
            /* seeds */
            info = strchr(buf, ':') + 2;
            seeds = 0;
            long seed;
            while (info && sscanf(info, "%ld", &seed)) {
                next_stage[seeds++] = seed;
                char *next = strchr(info, ' ');
                if (NULL == next) {
                    info = NULL;
                } else {
                    info = next + 1;
                }
                if (opts.debug) {
                    printf("Found seed %d: %ld\n", seeds, seed);
                }
            }
            break;
        default:
            if (sscanf(buf, "%ld %ld %ld", &dest_start, &source_start, &range_length) != 3) {
                /* Assume it's the headline */
                if (opts.debug) {
                    printf("Next up: %s\n", buf);
                }
                break;
            }
            if (opts.debug) {
                printf("Dest/source/length: %ld %ld %ld\n", dest_start, source_start, range_length);
            }
            /* Check all seeds, or soils, or whatever ... */
            for (int i = 0; i < seeds; i++) {
                if (all_seeds[i] == -1) {
                    continue;
                }
                if (source_start <= all_seeds[i] && all_seeds[i] <= source_start + range_length - 1) {
                    next_stage[i] = dest_start + (all_seeds[i] - source_start);
                    if (opts.debug) {
                        printf("  Mapping %ld to %ld\n", all_seeds[i], next_stage[i]);
                    }
                }
            }
            break;
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    long min_final = next_stage[0];
    for (int i = 1; i < seeds; i++) {
        if (next_stage[i] < min_final) {
            min_final = next_stage[i];
        }
    }
    printf("Minimum final value: %ld\n", min_final);
}

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
     %s - Program for AoC 2023 puzzles; Day 5, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 5, part 1.\n\
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
