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


int
compare_numbers(const void *a, const void *b)
{
    int i1, i2;

    i1 = *((int *)a);
    i2 = *((int *)b);
    if (i1 == i2) {
        return 0;
    }
    if (i1 < i2) {
        return -1;
    }
    return 1;
}

void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    int total_score = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        int card;
        char *info, *bar, *numbers;

        sscanf(buf, "Card %d:", &card);
        if (opts.debug) {
            printf("Reading card %d\n", card);
        }
        info = strchr(buf, ':') + 2;
        bar = strchr(buf, '|');
        numbers = bar + 2;
        *(bar - 1) = '\0';

        int total_winners = (strlen(info) + 1) / 3;
        int total_numbers = (strlen(numbers) + 1) / 3;
        int *winners_list = calloc(total_winners, sizeof(int));
        int *numbers_list = calloc(total_numbers, sizeof(int));

        for (int ix = 0; ix < total_winners; ix++) {
            int number;

            sscanf(info, "%d", &number);
            winners_list[ix] = number;
            info += 3;
        }
        for (int ix = 0; ix < total_numbers; ix++) {
            int number;

            sscanf(numbers, "%d", &number);
            numbers_list[ix] = number;
            numbers += 3;
        }
        qsort(winners_list, total_winners, sizeof(int), compare_numbers);
        qsort(numbers_list, total_numbers, sizeof(int), compare_numbers);
        int hits = 0;
        int score = 0;
        int ix_w = 0;
        int ix_n = 0;
        while (ix_w < total_winners && ix_n < total_numbers) {
            if (winners_list[ix_w] == numbers_list[ix_n]) {
                if (hits == 0) {
                    score = 1;
                } else {
                    score *= 2;
                }
                hits++;
                ix_w++;
                ix_n++;
            } else {
                if (winners_list[ix_w] < numbers_list[ix_n]) {
                    ix_w++;
                } else {
                    ix_n++;
                }
            }
        }
        if (opts.debug) {
            printf("Hits: %d; Score: %d\n", hits, score);
        }
        total_score += score;
        free(winners_list);
        free(numbers_list);
    }
    printf("Total score: %d\n", total_score);
    if (opts.debug) {
        printf("DEBUG: End of file\n");
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
     %s - Program for AoC 2023 puzzles; Day 4, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 4, part 1.\n\
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
