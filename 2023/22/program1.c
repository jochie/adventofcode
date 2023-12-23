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
# define DAY    22
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

typedef enum direction {
    X,
    Y,
    Z
} direction;

int max_x, max_y;

struct brick {
    int       id;
    int       x1, y1, z1;
    int       x2, y2, z2;
    /* Keep track of the direction the brick (if it's just one
     * position then that defaults to Z)
     */
    direction d;
    bool      zappable;
    /*
     * which other bricks are we supporting
     */
    int       supporting[10];
    int       t_supporting;
    /*
     * which bricks are supporting this one
     */
    int       supported_by[10];
    int       t_supported_by;
} bricks[1500];

int b_total;

int
compare_bricks(const void *a, const void *b)
{
    struct brick *brick1 = (struct brick *)a;
    struct brick *brick2 = (struct brick *)b;

    /*
     * ========  or  =====  or  ====
     * =====         =====      =======
     */
    if (brick1->z1 == brick2->z1) {
        if (brick1->z2 < brick2->z2) {
            return -1;
        }
        if (brick1->z2 == brick2->z2) {
            if (brick1->x1 < brick2->x1) {
                return -1;
            }
            if (brick1->x1 == brick2->x1) {
                if (brick1->y1 < brick2->y1) {
                    return -1;
                }
                if (brick1->y1 == brick2->y1) {
                    return 0;
                }
                return 1;
            }
            return 1;
        }
        return 1;
    }
    /*
     * =====   or  ======  or =======
     *   =====       ====       ====
     */
    if (brick1->z1 < brick2->z1) {
        return -1;
    }
    return 1;
}

void
dump_bricks()
{
    for (int i = b_total - 1; i >= 0; i--) {
        printf("Brick %4d (%c): (%d,%d,%3d) - (%d,%d,%3d) - %d\n",
               i + 1, 'A' + bricks[i].id,
               bricks[i].x1,
               bricks[i].y1,
               bricks[i].z1,
               bricks[i].x2,
               bricks[i].y2,
               bricks[i].z2,
               bricks[i].d);
    }
    return;

    int max_z = bricks[b_total - 1].z2;
    char side[400][20];

    /* Y view */
    for (int y = 0; y <= max_y; y++) {
        side[0][y] = '-';
    }
    side[0][max_y + 1] = '\0';
    for (int z = 1; z <= max_z; z++) {
        for (int y = 0; y <= max_y; y++) {
            side[z][y] = '.';
        }
        side[z][max_y + 1] = '\0';
    }
    for (int b = 0; b < b_total; b++) {
        switch (bricks[b].d) {
        case X:
            {
                int y = bricks[b].y1;
                int z = bricks[b].z1;
                if (side[z][y] == '.') {
                    side[z][y] = 'A' + bricks[b].id;
                } else {
                    side[z][y] = '?';
                }
            }
            break;
        case Y:
            {
                int z = bricks[b].z1;
                for (int y = bricks[b].y1; y <= bricks[b].y2; y++) {
                    if (side[z][y] == '.') {
                        side[z][y] = 'A' + bricks[b].id;
                    } else {
                        side[z][y] = '?';
                    }
                }
            }
            break;
        case Z:
            {
                int y = bricks[b].y1;
                for (int z = bricks[b].z1; z <= bricks[b].z2; z++) {
                    if (side[z][y] == '.') {
                        side[z][y] = 'A' + bricks[b].id;
                    } else {
                        side[z][y] = '?';
                    }
                }
            }
            break;
        }
    }
    printf(" Y\n");
    for (int z = max_z; z >= 0; z--) {
        printf("%s %d\n", side[z], z);
    }

    /* X view */
    for (int x = 0; x <= max_x; x++) {
        side[0][x] = '-';
    }
    side[0][max_x + 1] = '\0';
    for (int z = 1; z <= max_z; z++) {
        for (int x = 0; x <= max_x; x++) {
            side[z][x] = '.';
        }
        side[z][max_x + 1] = '\0';
    }
    for (int b = 0; b < b_total; b++) {
        switch (bricks[b].d) {
        case X:
            {
                int z = bricks[b].z1;
                for (int x = bricks[b].x1; x <= bricks[b].x2; x++) {
                    if (side[z][x] == '.') {
                        side[z][x] = 'A' + bricks[b].id;
                    } else {
                        side[z][x] = '?';
                    }
                }

            }
            break;
        case Y:
            {
                int x = bricks[b].x1;
                int z = bricks[b].z1;
                if (side[z][x] == '.') {
                    side[z][x] = 'A' + bricks[b].id;
                } else {
                    side[z][x] = '?';
                }
            }
            break;
        case Z:
            {
                int x = bricks[b].x1;
                for (int z = bricks[b].z1; z <= bricks[b].z2; z++) {
                    if (side[z][x] == '.') {
                        side[z][x] = 'A' + bricks[b].id;
                    } else {
                        side[z][x] = '?';
                    }
                }
            }
            break;
        }
    }
    printf(" X\n");
    for (int z = max_z; z >= 0; z--) {
        printf("%s %d\n", side[z], z);
    }
}

void
sort_bricks()
{
    qsort(bricks, b_total, sizeof(struct brick), compare_bricks);
}

void
lower_bricks()
{
    int heights[10][10];

    for (int x = 0; x <= max_x; x++) {
        for (int y = 0; y <= max_y; y++) {
            heights[x][y] = 0;
        }
    }
    for (int b = 0; b < b_total; b++) {
        int drop = 0;
        switch (bricks[b].d) {
        case X:
            {
                if (opts.debug) {
                    printf("Brick %d in X direction\n", b);
                }
                for (int x = bricks[b].x1; x <= bricks[b].x2; x++) {
                    int h = heights[x][bricks[b].y1];
                    if (h > drop) {
                        drop = h;
                    }
                }
                int d = bricks[b].z1 - drop - 1;
                if (d > 0) {
                    if (opts.debug) {
                        printf("Lowering brick %d (%c) by %d spots\n", b, 'A' + bricks[b].id, d);
                    }
                    /* Adjust height trackers */
                    bricks[b].z1 -= d;
                    bricks[b].z2 -= d;
                }
                for (int x = bricks[b].x1; x <= bricks[b].x2; x++) {
                    heights[x][bricks[b].y1] = bricks[b].z1;
                }
            }
            break;
        case Y:
            {
                if (opts.debug) {
                    printf("Brick %d in Y direction\n", b);
                }
                for (int y = bricks[b].y1; y <= bricks[b].y2; y++) {
                    int h = heights[bricks[b].x1][y];
                    if (h > drop) {
                        drop = h;
                    }
                }
                int d = bricks[b].z1 - drop - 1;
                if (d > 0) {
                    if (opts.debug) {
                        printf("Lowering brick %d (%c) by %d spots\n", b, 'A' + bricks[b].id, d);
                    }
                    bricks[b].z1 -= d;
                    bricks[b].z2 -= d;
                }
                for (int y = bricks[b].y1; y <= bricks[b].y2; y++) {
                    heights[bricks[b].x1][y] = bricks[b].z1;
                }
            }
            break;
        case Z:
            {
                if (opts.debug) {
                    printf("Brick %d in Z direction\n", b);
                }
                int h = heights[bricks[b].x1][bricks[b].y1];
                int d = bricks[b].z1 - h - 1;
                if (d > 0) {
                    if (opts.debug) {
                        printf("Lowering brick %d (%c) by %d spots\n", b, 'A' + bricks[b].id, d);
                    }
                    bricks[b].z1 -= d;
                    bricks[b].z2 -= d;
                    heights[bricks[b].x1][bricks[b].y1] = bricks[b].z2;
                }
            }
            break;
        }
        if (opts.debug) {
            printf("Brick %d lowered: (%d,%d,%d) - (%d,%d,%d)\n",
                   b,
                   bricks[b].x1, bricks[b].y1, bricks[b].z1,
                   bricks[b].x2, bricks[b].y2, bricks[b].z2);
            printf("Z grid:\n");
            char axis[20];
            for (int x = 0; x <= max_x; x++) {
                axis[x] = '0' + x;
            }
            axis[max_x + 1] = '\0';
            bool seen = false;
            printf("    %s\n\n", axis);
            for (int y = max_y; y >= 0; y--) {
                char buf[20];
                for (int x = 0; x <= max_x; x++) {
                    buf[x] = '0' + heights[x][y];
                }
                buf[max_x + 1] = '\0';
                if (!seen) {
                    printf("Y %d %s\n", y, buf);
                    seen = true;
                } else {
                    printf("  %d %s\n", y, buf);
                }
            }
            printf("\n");
        }
    }
}

bool
mark_zappable(int b, char *reason)
{
    if (bricks[b].zappable) {
        return false;
    }
    if (opts.debug) {
        printf("Marking brick %d (%c) as zappable: %s\n", b, 'A' + bricks[b].id, reason);
    }
    bricks[b].zappable = true;
    return true;
}

# define EMPTY -1

int occupied[20][20][400];

void
mark_occupied()
{
    for (int z = 0; z < 400; z++) {
        for (int x = 0; x <= max_x; x++) {
            for (int y = 0; y <= max_y; y++) {
                occupied[x][y][z] = EMPTY;
            }
        }
    }

    for (int b = 0; b < b_total; b++) {
        switch (bricks[b].d) {
        case X:
            for (int x = bricks[b].x1; x <= bricks[b].x2; x++) {
                occupied[x][bricks[b].y1][bricks[b].z1] = b;
            }
            break;
        case Y:
            for (int y = bricks[b].y1; y <= bricks[b].y2; y++) {
                occupied[bricks[b].x1][y][bricks[b].z1] = b;
            }
            break;
        case Z:
            for (int z = bricks[b].z1; z <= bricks[b].z2; z++) {
                occupied[bricks[b].x1][bricks[b].y1][z] = b;
            }
            break;
        }
    }
}

void
add_supporting(int b, int supports)
{
    /* Mark this in the brick itself */
    bool found = false;
    for (int i = 0; i < bricks[b].t_supporting; i++) {
        if (bricks[b].supporting[i] == supports) {
            found = true;
            break;
        }
    }
    if (!found) {
        bricks[b].supporting[bricks[b].t_supporting] = supports;
        bricks[b].t_supporting++;
    }

    /* Mark this in the brick it supports */
    found = false;
    for (int i = 0; i < bricks[supports].t_supported_by; i++) {
        if (bricks[supports].supported_by[i] == b) {
            found = true;
            break;
        }
    }
    if (!found) {
        bricks[supports].supported_by[bricks[supports].t_supported_by] = b;
        bricks[supports].t_supported_by++;
    }
}

/*
 * Go through the bricks and see which of them is supporting which
 * other bricks
 */
void
mark_supporting()
{
    for (int b = 0; b < b_total; b++) {
        int found;
        switch (bricks[b].d) {
        case X:
            for (int x = bricks[b].x1; x <= bricks[b].x2; x++) {
                found = occupied[x][bricks[b].y1][bricks[b].z1 + 1];
                if (found != EMPTY) {
                    add_supporting(b, found);
                }
            }
            break;
        case Y:
            for (int y = bricks[b].y1; y <= bricks[b].y2; y++) {
                found = occupied[bricks[b].x1][y][bricks[b].z1 + 1];
                if (found != EMPTY) {
                    add_supporting(b, found);
                }
            }
            break;
        case Z:
            found = occupied[bricks[b].x1][bricks[b].y1][bricks[b].z2 + 1];
            if (found != EMPTY) {
                add_supporting(b, found);
            }
            break;
        }
    }
}

void
dump_supporting_data()
{
    for (int b = 0; b < b_total; b++) {
        printf("Brick %d (%c):\n", b + 1, 'A' + bricks[b].id);
        for (int s = 0; s < bricks[b].t_supporting; s++) {
            printf("  Supports brick %d (%c)\n",
                   bricks[b].supporting[s] + 1,
                   'A' + bricks[bricks[b].supporting[s]].id);
        }
        for (int s = 0; s < bricks[b].t_supported_by; s++) {
            printf("  Supported by brick %d (%c)\n",
                   bricks[b].supported_by[s] + 1,
                   'A'+ bricks[bricks[b].supported_by[s]].id);
        }
    }
}

/*
 * We can zap the bricks that have nothing directly above them
 *
 * We can also zap the bricks below other bricks that are held up by
 * multiple bricks, but ONLY if that applies to all the bricks these
 * first bricks are supporting (that aspect I overlooked initially).
 */
int
zap_bricks()
{
    int zapped = 0;
    char buf[MAX_LEN];

    mark_occupied();
    mark_supporting();
    if (opts.debug) {
        dump_supporting_data();
    }

    for (int b = 0; b < b_total; b++) {
        if (bricks[b].t_supporting == 0) {
            sprintf(buf, "It's not supporting anything.");
            /* This particular one shouldn't lead to duplicates, but
               doesn't hurt to be consistent */
            if (mark_zappable(b, buf)) {
                zapped++;
            }
            continue;
        }
        bool zappable = true;
        /*
         * Confirm that all the bricks that this one is holding up, have more than one support.
         */
        for (int s = 0; s < bricks[b].t_supporting; s++) {
            if (bricks[bricks[b].supporting[s]].t_supported_by < 2) {
                zappable = false;
                break;
            }
        }
        if (zappable) {
            sprintf(buf, "All the bricks it's supporting have multiple supports.");
            if (mark_zappable(b, buf)) {
                zapped++;
            }
        }
    }
    return zapped;
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

    b_total = 0;
    max_x = 0;
    max_y = 0;
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
        int x1, y1, z1, x2, y2, z2;
        sscanf(buf, "%d,%d,%d~%d,%d,%d", &x1, &y1, &z1, &x2, &y2, &z2);
        if (opts.debug) {
            printf("(%d,%d,%d) - (%d,%d,%d)\n",
                   x1, y1, z1, x2, y2, z2);
        }
        if (x1 > x2 || y1 > y2 || z1 > z2) {
            printf("Out of order for this one?\n");
            exit(1);
        }
        if (x1 > max_x) {
            max_x = x1;
        }
        if (x2 > max_x) {
            max_x = x2;
        }
        if (y1 > max_y) {
            max_y = y1;
        }
        if (y2 > max_y) {
            max_y = y2;
        }
        bricks[b_total].id = b_total;
        bricks[b_total].x1 = x1;
        bricks[b_total].y1 = y1;
        bricks[b_total].z1 = z1;
        bricks[b_total].x2 = x2;
        bricks[b_total].y2 = y2;
        bricks[b_total].z2 = z2;
        bricks[b_total].zappable = false;
        bricks[b_total].t_supporting = 0;
        bricks[b_total].t_supported_by = 0;
        if (x1 < x2) {
            bricks[b_total].d = X;
        } else if (y1 < y2) {
            bricks[b_total].d = Y;
        } else {
            /* Could also be a single brick rather than an upright stack */
            bricks[b_total].d = Z;
        }
        b_total++;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (opts.debug) {
        printf("Found %d bricks, covering a grid of %d x %d\n", b_total, max_x + 1, max_y + 1);
    }
    sort_bricks();
    lower_bricks();
    if (opts.debug) {
        dump_bricks();
    }
    int zapped = zap_bricks();
    printf("Of the bricks, %d could be zapped.\n", zapped);
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
