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
# define DAY    25
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


# define MAX_PARTS  1500
# define MAX_TARGETS  20
# define MAX_NAME     10

struct part {
    char name[MAX_NAME];
    int  targets[MAX_TARGETS];
    int  t_targets;
    bool seen;
    int  depth;
} parts[MAX_PARTS];
int t_parts;

int cuts[10];
int t_cuts;

void
init_parts()
{
    t_parts = 0;
}

/*
 * Find or add the part entry, return the ID for it.
 */
int
add_part(char name[MAX_NAME])
{
    for (int i = 0; i < t_parts; i++) {
        if (!strcmp(parts[i].name, name)) {
            /* Already existed. */
            return i;
        }
    }
    if (t_parts == MAX_PARTS) {
        printf("Too many parts, cannot add %s.\n", name);
        exit(1);
    }
    strcpy(parts[t_parts].name, name);
    parts[t_parts].t_targets = 0;
    t_parts++;
    return t_parts - 1;
}

/*
 * Add the target entry, if necessary
 */
void
add_target(int p, int t)
{
    int t_t = parts[p].t_targets;
    for (int i = 0; i < t_t; i++) {
        if (parts[p].targets[i] == t) {
            /* Already there. */
            return;
        }
    }
    parts[p].targets[t_t] = t;
    parts[p].t_targets++;
}

void
dump_parts()
{
    for (int p = 0; p < t_parts; p++) {
        printf("%s:", parts[p].name);
        for (int t = 0; t < parts[p].t_targets; t++) {
            int t_i = parts[p].targets[t];
            printf(" %s", parts[t_i].name);
        }
        printf("\n");
    }
}

/*
 * Clear the 'seen' flag for all parts
 */
void
mark_unseen()
{
    for (int p = 0; p < t_parts; p++) {
        parts[p].seen = false;
    }
}

int broken[3][2];

/*
 * Check if we marked this edge as broken for this run
 */
bool
edge_broken(int p1, int p2)
{
    for (int i = 0; i < 3; i++) {
        if (p1 == broken[i][0] && p2 == broken[i][1]) {
            return true;
        }
        if (p1 == broken[i][1] && p2 == broken[i][0]) {
            return true;
        }
    }
    return false;
}

/*
 * Data-structure and functions that maintain the queue for the breadth-first searches below
 */

int queue[MAX_PARTS];
int t_queue;

void
queue_init()
{
    t_queue = 0;
}

void
queue_add(int p)
{
    /* Check for dupes */
    for (int i = 0; i < t_queue; i++) {
        if (queue[i] == p) {
            return;
        }
    }
    if (t_queue == MAX_PARTS) {
        printf("Need more space on the queue.\n");
        exit(1);
    }
    queue[t_queue] = p;
    t_queue++;
}

/*
 * Breadth-First Search to calculate how many steps to cover the
 * entire network
 */
int
calculate_network_distance(int p)
{
    mark_unseen();
    int d = 0;

    queue_init();
    queue_add(p);
    parts[p].seen = true;

    while (t_queue > 0) {
        int copy[MAX_PARTS];
        int t_copy = t_queue;
        queue_init();
        memcpy(copy, queue, sizeof(queue));
        for (int i = 0; i < t_copy; i++) {
            int i_p = copy[i];
            for (int t = 0; t < parts[i_p].t_targets; t++) {
                int f = parts[i_p].targets[t];
                if (parts[f].seen) {
                    continue;
                }
                parts[f].seen = true;
                queue_add(f);
            }
        }
        d++;
    }
    parts[p].depth = d;
    return d;
}

/*
 * Breadth-First Search to calculate the size of network (partition),
 * taking into account the edges that have been marked as broken.
 */
int
calculate_network_size(int p)
{
    mark_unseen();
    int count = 1;
    queue_init();
    queue_add(p);
    parts[p].seen = true;

    while (t_queue > 0) {
        int copy[MAX_PARTS];
        int t_copy = t_queue;
        queue_init();
        memcpy(copy, queue, sizeof(queue));
        for (int i = 0; i < t_copy; i++) {
            int i_p = copy[i];
            for (int t = 0; t < parts[i_p].t_targets; t++) {
                int f = parts[i_p].targets[t];
                if (edge_broken(i_p, f)) {
                    continue;
                }
                if (parts[f].seen) {
                    continue;
                }
                parts[f].seen = true;
                count++;
                queue_add(f);
            }
        }
    }
    return count;
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

    init_parts();
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
        char name[10];
        int len = 0;
        sscanf(buf, "%[a-z]%n:", name, &len);
        if (opts.debug) {
            printf("Detected component '%s' - len = %d\n", name, len);
        }

        int p = add_part(name);

        char *info = buf + len + 2;
        while (strlen(info)) {
            char target[10];
            len = 0;
            sscanf(info, "%[a-z]%n", target, &len);
            int t = add_part(target);
            add_target(p, t);
            add_target(t, p);
            if (opts.debug) {
                printf("  Target '%s' - len = %d\n", target, len);
            }
            if (info[len] == '\0') {
                break;
            }
            info = info + len + 1;
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    if (opts.debug || opts.verbose) {
        printf("Total parts parsed: %d\n", t_parts);
    }
    if (opts.debug) {
        dump_parts();
    }

    int min_depth = -1;
    for (int p = 0; p < t_parts; p++) {
        int d = calculate_network_distance(p);
        if (min_depth == -1 || d < min_depth) {
            min_depth = d;
        }
        if (opts.debug) {
            printf("Network depth for %s: %d\n",
                   parts[p].name, d);
        }
    }
    if (opts.debug || opts.verbose) {
        printf("Prime targets (with the lowest network depth of %d):\n\n", min_depth);
    }
    t_cuts = 0;
    for (int p = 0; p < t_parts; p++) {
        if (parts[p].depth == min_depth) {
            if (opts.debug) {
                printf("%s:", parts[p].name);
            }
            for (int t = 0; t < parts[p].t_targets; t++) {
                int p_t = parts[p].targets[t];
                if (opts.debug) {
                    printf(" %s", parts[p_t].name);
                }
            }
            if (opts.debug) {
                printf("\n");
            }
            cuts[t_cuts] = p;
            t_cuts++;
        }
    }
    /*
     * Look at all permutations of selecting three edges from the
     * selected prime targets
     */
    int perms = 0;
    for (int c1 = 0; c1 < t_cuts; c1++) {
        int p1 = cuts[c1];
        for (int c2 = c1 + 1; c2 < t_cuts; c2++) {
            int p2 = cuts[c2];
            for (int c3 = c2 + 1; c3 < t_cuts; c3++) {
                int p3 = cuts[c3];
                perms++;
                if (opts.debug) {
                    printf("Permutation %d: %s, %s, %s\n",
                           perms,
                           parts[p1].name,
                           parts[p2].name,
                           parts[p3].name);
                }
                for (int t1 = 0; t1 < parts[p1].t_targets; t1++) {
                    int p4 = parts[p1].targets[t1];
                    if (p4 == p2 || p4 == p3) {
                        continue;
                    }
                    for (int t2 = 0; t2 < parts[p2].t_targets; t2++) {
                        int p5 = parts[p2].targets[t2];
                        if (p5 == p1 || p5 == p3 || p5 == p4) {
                            continue;
                        }
                        for (int t3 = 0; t3 < parts[p3].t_targets; t3++) {
                            int p6 = parts[p3].targets[t3];
                            if (p6 == p1 || p6 == p2 || p6 == p4 || p6 == p5) {
                                continue;
                            }
                            broken[0][0] = p1;
                            broken[0][1] = p4;
                            broken[1][0] = p2;
                            broken[1][1] = p5;
                            broken[2][0] = p3;
                            broken[2][1] = p6;
                            int count = calculate_network_size(p1);
                            if (opts.debug) {
                                printf("Testing cutting these wires:\n");
                                printf("  %s - %s\n", parts[p1].name, parts[p4].name);
                                printf("  %s - %s\n", parts[p2].name, parts[p5].name);
                                printf("  %s - %s\n", parts[p3].name, parts[p6].name);
                                printf("Network (partition) size: %d\n", count);
                            }
                            if (count < t_parts) {
                                printf("Cutting these wires does the trick:\n");
                                printf("  %s - %s\n", parts[p1].name, parts[p4].name);
                                printf("  %s - %s\n", parts[p2].name, parts[p5].name);
                                printf("  %s - %s\n", parts[p3].name, parts[p6].name);
                                printf("\n");
                                printf("%d * %d = %d\n", count, t_parts - count, count * (t_parts - count));
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
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
