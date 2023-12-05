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
# define MAX_RANGES 100

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
    struct {
        long long start;
        long long end;
    } all_seeds[MAX_RANGES], next_stage[MAX_RANGES];
    int seeds = 0;
    int next_seeds = 0;
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
                /* Copy the ones that haven't been mapped somehow */
                if (all_seeds[i].start == -1 && all_seeds[i].end == -1) {
                    continue;
                }
                if (next_seeds == MAX_RANGES) {
                    printf("ERROR: Not enough ranges to handle this (#1).\n");
                    exit(1);
                }
                next_stage[next_seeds] = all_seeds[i];
                next_seeds++;
            }
            for (int i = 0; i < next_seeds; i++) {
                if (opts.debug) {
                    printf("Range: %lld-%lld\n", next_stage[i].start, next_stage[i].end);
                }
                all_seeds[i] = next_stage[i];
            }
            seeds = next_seeds;
            next_seeds = 0;
            continue;
        }
        char *info;
        long long dest_start, dest_end, source_start, range_length;
        switch (state) {
        case 0:
            /* seeds */
            info = strchr(buf, ':') + 2;
            next_seeds = 0;
            long long start, length;
            while (info && sscanf(info, "%lld %lld", &start, &length)) {
                next_stage[next_seeds].start = start;
                next_stage[next_seeds].end = start + length -1;
                next_seeds++;
                char *next = strchr(info, ' ');
                if (NULL == next) {
                    info = NULL;
                } else {
                    info = next + 1;
                    next = strchr(info, ' ');
                    if (NULL == next) {
                        info = NULL;
                    } else {
                        info = next + 1;
                    }
                }
                if (opts.debug) {
                    printf("Found seed range %d: %lld-%lld\n", next_seeds, start, start + length - 1);
                }
            }
            break;
        default:
            if (sscanf(buf, "%lld %lld %lld", &dest_start, &source_start, &range_length) != 3) {
                /* Assume it's the headline */
                if (opts.debug) {
                    printf("Next up: %s\n", buf);
                }
                break;
            }
            dest_end = dest_start + range_length - 1;
            if (opts.debug) {
                printf("Dest/source/length: %lld %lld %lld a.k.a %lld-%lld maps to %lld-%lld\n",
                       dest_start, source_start, range_length,
                       source_start,
                       source_start + range_length - 1,
                       dest_start,
                       dest_end
                       );
            }
            /*
             * Check all seeds, or soils, or whatever ...
             *
             * Start at the end, so that we can add more entries
             * during the for() loop itself of ranges/segments that
             * might be mapped still by another line
             */
            for (int i = seeds - 1; i >= 0; i--) {
                if (all_seeds[i].start == -1 && all_seeds[i].end == -1) {
                    continue;
                }
                long long source_end = source_start + range_length - 1;
                if (source_start <= all_seeds[i].start && all_seeds[i].end <= source_end) {
                    /*
                     * Completely contained
                     *
                     * <================ source =============>
                     *            <seed-range>
                     */
                    if (opts.debug) {
                        printf("  (0) %lld-%lld entirely within the range.\n", all_seeds[i].start, all_seeds[i].end);
                    }
                    if (next_seeds == MAX_RANGES) {
                        printf("ERROR: Not enough ranges to handle this (#2).\n");
                        exit(1);
                    }
                    next_stage[next_seeds].start = dest_start + (all_seeds[i].start - source_start);
                    next_stage[next_seeds].end = dest_start + (all_seeds[i].end - source_start);
                    if (opts.debug) {
                        printf("  Mapped to %lld-%lld\n", next_stage[next_seeds].start, next_stage[next_seeds].end);
                    }
                    all_seeds[i].start = -1;
                    all_seeds[i].end = -1;
                    next_seeds++;
                } else if (source_start <= all_seeds[i].start && all_seeds[i].start <= source_end) {
                    /*
                     * Partially contained
                     *
                     *        <========= source ======>
                     *                <======seed-range=======>
                     */
                    if (opts.debug) {
                        printf("  (1) %lld-%lld partial overlap.\n", all_seeds[i].start, all_seeds[i].end);
                    }
                    if (next_seeds == MAX_RANGES) {
                        printf("ERROR: Not enough ranges to handle this (#3).\n");
                        exit(1);
                    }
                    next_stage[next_seeds].start = dest_start + (all_seeds[i].start - source_start);
                    next_stage[next_seeds].end   = dest_start + (source_end - source_start);
                    if (opts.debug) {
                        printf("  Mapped to %lld-%lld\n", next_stage[next_seeds].start, next_stage[next_seeds].end);
                    }
                    next_seeds++;
                    /* Reduce the range that may still need to be mapped */
                    all_seeds[i].start = source_end + 1;
                    if (opts.debug) {
                        printf("  Remainder %lld-%lld\n", all_seeds[i].start, all_seeds[i].end);
                    }
                } else if (source_start <= all_seeds[i].end && all_seeds[i].end <= source_end) {
                    /*
                     * Partially contained
                     *
                     *            <========= source ======>
                     *    <======seed-range=======>
                     */
                    if (opts.debug) {
                        printf("  (2) %lld-%lld partial overlap.\n", all_seeds[i].start, all_seeds[i].end);
                    }
                    if (next_seeds == MAX_RANGES) {
                        printf("ERROR: Not enough ranges to handle this (#4).\n");
                        exit(1);
                    }
                    next_stage[next_seeds].start = dest_start;
                    next_stage[next_seeds].end = dest_start + (all_seeds[i].end - source_start);
                    if (opts.debug) {
                        printf("  Mapped to %lld-%lld\n", next_stage[next_seeds].start, next_stage[next_seeds].end);
                    }
                    next_seeds++;
                    /* Reduce the range that may still need to be mapped */
                    all_seeds[i].end = source_start - 1;
                    if (opts.debug) {
                        printf("  Remainder %lld-%lld\n", all_seeds[i].start, all_seeds[i].end);
                    }
                } else if (all_seeds[i].start <= source_start && source_end <= all_seeds[i].end) {
                    /*
                     * Completely contained the other way around
                     *
                     * <========seed-range========>
                     *     <==== source ===>
                     */
                    if (opts.debug) {
                        printf("   (3) %lld-%lld entirely envelops this range.\n", all_seeds[i].start, all_seeds[i].end);
                    }
                    if (next_seeds == MAX_RANGES) {
                        printf("ERROR: Not enough ranges to handle this (#5).\n");
                        exit(1);
                    }
                    next_stage[next_seeds].start = dest_start;
                    next_stage[next_seeds].end = dest_end;
                    if (opts.debug) {
                        printf("  Mapped to %lld-%lld\n", next_stage[next_seeds].start, next_stage[next_seeds].end);
                    }
                    next_seeds++;

                    if (seeds == MAX_RANGES) {
                        printf("ERROR: Not enough ranges to handle this (#6).\n");
                        exit(1);
                    }
                    /* Add the tail end at the end of the list */
                    all_seeds[seeds].start = source_end + 1;
                    all_seeds[seeds].end = all_seeds[i].end;
                    if (opts.debug) {
                        printf("  Tail remainder %lld-%lld\n", all_seeds[seeds].start, all_seeds[seeds].end);
                    }
                    seeds++;
                    all_seeds[i].end = source_start - 1;
                    if (opts.debug) {
                        printf("  Head remainder %lld-%lld\n", all_seeds[i].start, all_seeds[i].end);
                    }
                }
            }
            break;
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    for (int i = 0; i < seeds; i++) {
        /* Copy the ones that haven't been mapped somehow */
        if (all_seeds[i].start == -1 && all_seeds[i].end == -1) {
            continue;
        }
        if (next_seeds == MAX_RANGES) {
            printf("ERROR: Not enough ranges to handle this (#7).\n");
            exit(1);
        }
        next_stage[next_seeds] = all_seeds[i];
        next_seeds++;
    }
    
    long long min_final = next_stage[0].start;
    for (int i = 1; i < next_seeds; i++) {
        if (next_stage[i].start < min_final) {
            min_final = next_stage[i].start;
        }
    }
    printf("Minimum final value: %lld\n", min_final);
    if (opts.debug) {
        printf("Final number of ranges: %d\n", next_seeds);
    }
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
     %s - Program for AoC 2023 puzzles; Day 5, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 5, part 2.\n\
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
