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

# define USE_MATH

# ifdef USE_MATH
# include <math.h>       /* sqrt()               */
# endif

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

    struct {
        int duration;
        int distance;
    } races[4];

    fgets(buf, MAX_LEN, fd);
    /* Strip the newline, if present */
    if (buf[strlen(buf) - 1] == '\n') {
        buf[strlen(buf) - 1] = '\0';
    }
    if (opts.debug) {
        printf("DEBUG: Line received: '%s'\n", buf);
    }
    int i1, i2, i3, i4;
    sscanf(buf, "Time: %d %d %d %d",
           &races[0].duration,
           &races[1].duration,
           &races[2].duration,
           &races[3].duration);

    fgets(buf, MAX_LEN, fd);
    /* Strip the newline, if present */
    if (buf[strlen(buf) - 1] == '\n') {
        buf[strlen(buf) - 1] = '\0';
    }
    if (opts.debug) {
        printf("DEBUG: Line received: '%s'\n", buf);
    }
    int race_total = sscanf(buf, "Distance: %d %d %d %d",
                            &races[0].distance,
                            &races[1].distance,
                            &races[2].distance,
                            &races[3].distance);
    int multiplied = 1;
    for (int i = 0; i < race_total; i++) {
        int winners = 0;
        if (opts.debug) {
            printf("Race %d: %d ms; %d mm\n", i + 1, races[i].duration, races[i].distance);
        }
# ifdef USE_MATH
        /*
         * Solving this equation:
         *
         * (dist - w) * w = dura
         * dist * w - w^2 - dura = 0
         * - w ^ 2 + dist * w - dura = 0
         *
         * That's of the form: a x ^2 + b x + c = 0
         *
         * https://en.wikipedia.org/wiki/Quadratic_equation
         *
         * a = -1
         * b = dist
         * c = -dura
         *
         * x = (-b +/- sqrt(b^2 - 4ac)) / (2a)
         */
        double a, b, c, s, x1, x2;

        a = -1;
        b = (double)races[i].duration;
        c = -(double)races[i].distance;
        s = sqrt(b * b - 4 * a * c);
        x1 = (-b + s) / (2 * a);
        x2 = (-b - s) / (2 * a);
        x1 = floor(x1) + 1;
        if (x2 - floor(x2) < 0.0001) {
            x2 = floor(x2) - 1;
        } else {
            x2 = floor(x2);
        }
        double winners_alt = x2 - x1 + 1;
        winners = (int)winners_alt;
# else
        for (int wait = 1; wait < races[i].duration; wait++) {
            int distance = (races[i].duration - wait) * wait;
            if (opts.debug) {
                printf("Wait %d ms - travel %d mm\n", wait, distance);
            }
            if (distance > races[i].distance) {
                winners++;
            }
        }
# endif
        if (opts.debug) {
            printf("Ways to win: %d\n", winners);
        }
        multiplied *= winners;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Final multiplier: %d\n", multiplied);
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
     %s - Program for AoC 2023 puzzles; Day 6, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 6, part 1.\n\
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
