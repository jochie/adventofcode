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

# include <openssl/sha.h>

# define YEAR 2023
# define DAY    24
# define PART    1

# define STR(x) _STR(x)
# define _STR(x) #x

# define MAX_LEN 1024

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


struct hailstone {
    long px, py, pz;
    long vx, vy, vz;
} hailstones[400];
int t_hailstones;


bool
check_paths(int h1, int h2, long xy_min, long xy_max)
{
    /*
     *  / px1 \     / vx1 \         / px2 \     / vx2 \
     * (  py1  ) + (  vy1  ) * s = (  py2  ) + (  vy2  ) * t
     *  \ pz1 /     \ vz1 /         \ pz2 /     \ vz2 /
     *
     *    px1 + vx1 * s = px2 + vx2 * t
     *    py1 + vy1 * s = py2 + vy2 * t
     * =>
     *    vx1 * s = px2 + vx2 * t - px1
     *    vy1 * s = py2 + vy2 * t - py1
     * =>
     *    s = (px2 + vx2 * t - px1) / vx1
     *    s = (py2 + vy2 * t - py1) / vy1
     * =>
     *    (px2 + vx2 * t - px1) / vx1 = (py2 + vy2 * t - py1) / vy1
     * =>
     *    (px2 + vx2 * t - px1) * vy1 = (py2 + vy2 * t - py1) * vx1
     * =>
     *    px2 * vy1 + vx2 * t * vy1 - px1 * vy1 = py2 * vx1 + vy2 * t * vx1 - py1 * vx1
     * =>
     *    vx2 * t * vy1 - vy2 * t * vx1 = py2 * vx1 - py1 * vx1 - px2 * vy1 + px1 * vy1
     * =>
     *    t * (vx2 * vy1 - vy2 * vx1) = py2 * vx1 - py1 * vx1 - px2 * vy1 + px1 * vy1
     * =>
     *    t = (py2 * vx1 - py1 * vx1 - px2 * vy1 + px1 * vy1) / (vx2 * vy1 - vy2 * vx1)
     *
     * =====
     *
     *    px1 + vx1 * s = px2 + vx2 * t
     * =>
     *    vx1 * s = px2 + vx2 * t - px1
     * =>
     *    s = (px2 + vx2 * t - px1) / vx1
     */

    double px1 = (double)hailstones[h1].px;
    double py1 = (double)hailstones[h1].py;
    double px2 = (double)hailstones[h2].px;
    double py2 = (double)hailstones[h2].py;
    double vx1 = (double)hailstones[h1].vx;
    double vy1 = (double)hailstones[h1].vy;
    double vx2 = (double)hailstones[h2].vx;
    double vy2 = (double)hailstones[h2].vy;

    double div = (vx2 * vy1 - vy2 * vx1);
    if (div == 0) {
        if (opts.debug) {
            printf("Storms %d and %d never intersect.\n", h1 + 1, h2 + 1);
        }
        return false;
    }
    double t = (py2 * vx1 - py1 * vx1 - px2 * vy1 + px1 * vy1) / div;
    if (opts.debug) {
        printf("t = %f\n", t);
    }
    double x2 = px2 + vx2 * t;
    double y2 = py2 + vy2 * t;
    if (opts.debug) {
        printf("(X2,Y2) = (%f,%f)\n", x2, y2);
    }

    double s = (px2 + vx2 * t - px1) / vx1;
    if (opts.debug) {
        printf("s = %f\n", s);
    }
    double x1 = px1 + vx1 * s;
    double y1 = py1 + vy1 * s;
    if (opts.debug) {
        printf("(X1,Y1) = (%f,%f)\n", x1, y1);
    }

    if (t < 0 && s < 0) {
        if (opts.debug) {
            printf("They crossed, but in the past for both.\n");
        }
        return false;
    }
    if (t < 0) {
        if (opts.debug) {
            printf("They crossed, but it was in the past for storm %d.\n", h2 + 1);
        }
        return false;
    }
    if (s < 0) {
        if (opts.debug) {
            printf("They crossed, but it was in the past for storm %d.\n", h1 + 1);
        }
        return false;
    }
    if ((double)xy_min <= x1 && x1 <= (double)xy_max &&
        (double)xy_min <= y1 && y1 <= (double)xy_max) {
        if (opts.debug) {
            printf("(%f,%f) is in the right grid.\n", x1, y1);
        }
        return true;
    }
    return false;
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

    t_hailstones = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            unsigned char digest[SHA256_DIGEST_LENGTH];
            char hexdigest[SHA256_DIGEST_LENGTH * 2 + 1];

            SHA256((unsigned char *)buf, strlen(buf), digest);

            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                sprintf(&hexdigest[i * 2], "%02x", (unsigned int)digest[i]);
            }

            printf("DEBUG: Line received: [%s] '%s'\n", hexdigest, buf);
        }
        long x1, x2, x3, x4, x5, x6;
        if (sscanf(buf, "%ld, %ld, %ld @ %ld, %ld, %ld",
                   &x1, &x2, &x3, &x4, &x5, &x6) == 6) {
            hailstones[t_hailstones].px = x1;
            hailstones[t_hailstones].py = x2;
            hailstones[t_hailstones].pz = x3;
            hailstones[t_hailstones].vx = x4;
            hailstones[t_hailstones].vy = x5;
            hailstones[t_hailstones].vz = x6;
            t_hailstones++;
        } else {
            printf("Failed to parse input line: %s\n", buf);
            exit(1);
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (opts.debug) {
        printf("Parsed %d hailstones from the input.\n", t_hailstones);
    }
    long xy_min = 7;
    long xy_max = 27;
    if (t_hailstones > 5) {
        /* Assume it's the real input file */
        xy_min = 200000000000000;
        xy_max = 400000000000000;
    }
    int crossed = 0;
    if (opts.debug) {
        printf("Looking for paths crossing for (x,y) between [%ld,%ld]\n", xy_min, xy_max);
    }
    for (int s = 0; s < t_hailstones; s++) {
        for (int t = s + 1; t < t_hailstones; t++) {
            if (opts.debug) {
                printf("Comparing storm %d and %d\n", s + 1, t + 1);
            }
            if (check_paths(s, t, xy_min, xy_max)) {
                crossed++;
            }
        }
    }
    printf("%d of the paths will cross in the test range.\n", crossed);
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
