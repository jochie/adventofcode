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
    char lr[300];

    struct node {
        char n[4];
        char l[4];
        int  l_i;
        char r[4];
        int  r_i;
    } nodes[800];

    /* get the first line with left-right directions */
    fgets(lr, MAX_LEN, fd);
    if (lr[strlen(lr) - 1] == '\n') {
        lr[strlen(lr) - 1] = '\0';
    }

    if (opts.debug) {
        printf("Left/right instructions: %s\n", lr);
    }

    /* skip the empty line */
    fgets(buf, MAX_LEN, fd);

    /* read the node connections */
    int total = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        sscanf(buf, "%[A-Z0-9] = (%[A-Z0-9], %[A-Z0-9])", nodes[total].n, nodes[total].l, nodes[total].r);
        nodes[total].l_i = -1;
        nodes[total].r_i = -1;
        if (opts.debug) {
            printf("Node %s points to %s and %s\n", nodes[total].n, nodes[total].l, nodes[total].r);
        }
        total++;
    }
    if (opts.debug) {
        printf("Parsed %d nodes total.\n", total);
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }

    /* Resolve the l_i and r_i values, to speed things up? */
    for (int i = 0; i < total; i++) {
        for (int j = 0; j < total; j++) {
            if (!strcmp(nodes[i].n, nodes[j].l)) {
                nodes[j].l_i = i;
            }
            if (!strcmp(nodes[i].n, nodes[j].r)) {
                nodes[j].r_i = i;
            }
        }
    }
    int current_node = 0;
    int lr_index = 0;
    long steps = 0;

    struct a_node {
        int i;
        int f;
        int s;
    } a_nodes[10];
    int a_total = 0;
    for (int i = 0; i < total; i++) {
        if (nodes[i].n[2] == 'A') {
            a_nodes[a_total].i = i;
            a_nodes[a_total].f = -1;
            a_nodes[a_total].s = -1;
            a_total++;
            if (opts.debug) {
                printf("Found A node #%d at %d: %s\n", a_total, i, nodes[i].n);
            }
        }
    }

    int a_nodes_done = 0;

    while (true) {
        bool all_on_z = true;
        for (int i = 0; i < a_total; i++) {
            if (nodes[a_nodes[i].i].n[2] == 'Z') {
                if (opts.debug) {
                    printf("A node %d is on %s after %ld steps\n", i, nodes[a_nodes[i].i].n, steps);
                }
                if (a_nodes[i].f == -1) {
                    a_nodes[i].f = steps;
                } else if (a_nodes[i].s == -1) {
                    a_nodes[i].s = steps;
                } else {
                    a_nodes_done++;
                }
            } else {
                all_on_z = false;
            }
        }
        /*
         * Checking for all_on_z really only works for the sample
         * file. Based on an observation of debug output at an earlier
         * stage, we know that the various nodes ending in A keep
         * landing on the same nodes ending in Z, over and over, so we
         * need to know how long their cycles are, and keep going
         * until they've all been there twice.
         */
        if (all_on_z || a_nodes_done == a_total) {
            break;
        }

        if (opts.debug) {
            printf("Going %c next\n", lr[lr_index]);
        }
        /* move all nodes along */
        for (int i = 0; i < a_total; i++) {
            char *node_to_find;
            int next_node;

            int current_node = a_nodes[i].i;
            if (lr[lr_index] == 'L') {
                node_to_find = nodes[current_node].l;
                next_node    = nodes[current_node].l_i;
            } else {
                node_to_find = nodes[current_node].r;
                next_node    = nodes[current_node].r_i;
            }
            if (opts.debug) {
                printf("Next node: %s (%d)\n", node_to_find, next_node);
            }
            if (next_node == -1) {
                printf("Something went wrong? We looked for %s but could not find it.\n",
                       node_to_find);
                exit(1);
            }
            if (opts.debug) {
                printf("Index of %s is %d\n", node_to_find, next_node);
            }
            a_nodes[i].i = next_node;
        }
        steps++;
        lr_index = (lr_index + 1) % strlen(lr);
        if (opts.debug && steps % 1000 == 0) {
            printf("Steps so far: %ld\n", steps);
        }
    }

    /*
     * Now, knowing the cycle, we can take much bigger steps and don't
     * need to move around the real nodes any longer.
     *
     * This is still overly generic. In the actual input I was given,
     * two things were later apparent:
     *
     * 1. Each of the A nodes took as many steps to reach the Z node
     *    the first time, as the second time, so we could actually
     *    simplify this particular case with a least common multiple
     *    of the 6 numbers.
     * 2. All 6 of the A nodes have a cycle that is a multiple of 277.
     *
     * So the answer to the question is then:
     *
     * >>> 277 * (16343 / 277) * (11911 / 277) * (20221 / 277) *
     *     (21883 / 277) * (13019 / 277) * (19667 / 277)
     * 13524038372771
     */
    struct a_step {
        long steps;
        long incr;
    } a_steps[10];
    for (int i = 0; i < a_total; i++) {
        if (opts.debug) {
            printf("A node arrived at Z nodes %d first, %d second (cycle is %d).\n",
                   a_nodes[i].f,
                   a_nodes[i].s,
                   a_nodes[i].s - a_nodes[i].f);
        }
        a_steps[i].steps = a_nodes[i].f;
        a_steps[i].incr = a_nodes[i].s - a_nodes[i].f;
    }
    while (true) {
        bool done = true;

        long steps = a_steps[0].steps;
        long max_steps = a_steps[0].steps;
        for (int i = 1; i < a_total; i++) {
            if (a_steps[i].steps != steps) {
                done = false;
            }
            if (a_steps[i].steps > max_steps) {
                max_steps = a_steps[i].steps;
            }
        }
        if (done) {
            printf("Reached all **Z nodes after %ld steps.\n", steps);
            exit(0);
        }
        for (int i = 0; i < a_total; i++) {
            if (a_steps[i].steps < max_steps) {
                a_steps[i].steps += a_steps[i].incr;
            }
        }
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
     %s - Program for AoC 2023 puzzles; Day 8, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 8, part 2.\n\
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
