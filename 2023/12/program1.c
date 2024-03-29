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


bool
valid_grouping(char record[MAX_LEN + 1], int group_total, int grouping[10])
{
    int detected[10];
    int found = 0;
    bool is_damage = record[0] == '#';
    int damage_start;

    if (is_damage) {
        damage_start = 0;
    }
    for (int i = 1; i < strlen(record); i++) {
        if (record[i] == '#') {
            if (!is_damage) {
                is_damage = true;
                damage_start = i;
            }
        } else {
            /* Must be '.' */
            if (is_damage) {
                detected[found] = i - damage_start;
                found++;
                is_damage = false;
            }
        }
    }
    /* there may be damage at the end */
    if (is_damage) {
        detected[found] = strlen(record) - damage_start;
        found++;
    }
    if (found != group_total) {
        return false;
    }
    bool matched = true;
    for (int i = 0; i < found; i++) {
        if (detected[i] != grouping[i]) {
            matched = false;
        }
    }
    return matched;
}

int
valid_permutations(char record[MAX_LEN + 1], int group_total, int grouping[10])
{
    char *unknown;

    unknown = strchr(record, '?');
    if (NULL == unknown) {
        /* No (more) unknown found, is this a match for the groupings? */
        if (valid_grouping(record, group_total, grouping)) {
            return 1;
        }
        return 0;
    }
    /* printf("Found unknown at position %ld\n", unknown - record); */
    int found = 0;
    unknown[0] = '#';
    found += valid_permutations(record, group_total, grouping);
    unknown[0] = '.';
    found += valid_permutations(record, group_total, grouping);
    unknown[0] = '?';
    return found;
}

void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    int perm_total = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        char record[MAX_LEN + 1], groups[MAX_LEN + 1];
        int grouping[10];
        sscanf(buf, "%[?.#] %[0-9,]", record, groups);
        if (opts.debug) {
            printf("Record '%s' and groups '%s'\n", record, groups);
        }
        int group_total = 0;
        char *cur = groups;
        while (true) {
            int n, len;

            if (sscanf(cur, "%d%n", &n, &len)) {
                grouping[group_total] = n;
                group_total++;
            }
            cur += len;
            if (cur[0] != ',') {
                break;
            }
            cur++;
        }
        int perm = valid_permutations(record, group_total, grouping);
        if (opts.debug) {
            printf("  #Arrangements: %d\n", perm);
        }
        perm_total += perm;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Total #arrangements for this set: %d\n", perm_total);
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
     %s - Program for AoC 2023 puzzles; Day 12, part 1\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 12, part 1.\n\
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
