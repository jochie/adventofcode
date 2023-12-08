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
        sscanf(buf, "%[A-Z] = (%[A-Z], %[A-Z])", nodes[total].n, nodes[total].l, nodes[total].r);
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
    int current_node = 0;
    int lr_index = 0;
    int steps = 0;
    while (strcmp(nodes[current_node].n, "ZZZ")) {
        char *node_to_find;

        if (opts.debug) {
            printf("Going %c next\n", lr[lr_index]);
        }

        if (lr[lr_index] == 'L') {
            node_to_find = nodes[current_node].l;
        } else {
            node_to_find = nodes[current_node].r;
        }
        if (opts.debug) {
            printf("Next node: %s\n", node_to_find);
        }
        int next_node = -1;
        for (int i = 0; i < total; i++) {
            if (!strcmp(nodes[i].n, node_to_find)) {
                next_node = i;
                break;
            }
        }
        if (next_node == -1) {
            printf("Something went wrong? We looked for %s but could not find it.\n",
                   node_to_find);
            exit(1);
        }
        if (opts.debug) {
            printf("Index of %s is %d\n", node_to_find, next_node);
        }
        current_node = next_node;
        steps++;
        lr_index = (lr_index + 1) % strlen(lr);
    }
    printf("Steps to get to ZZZ: %d\n", steps);
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
     %s - Program for AoC 2023 puzzles; Day 8, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 8, part 1.\n\
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
