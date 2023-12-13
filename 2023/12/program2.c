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


char record[MAX_LEN + 1];
int group_total;
int grouping[50];
int needed[50];

/* Keep track of previously calculated numbers of permutations in this */
long permutations[50][MAX_LEN + 1];

long
map_groups(int pos, int found, char rec[MAX_LEN + 1])
{
    long orig_pos = pos;
    long perm = 0;

    /*
     * Check cache
     */
    if (permutations[found][pos] >= 0) {
        if (opts.debug) {
            printf("Cache hit for group %d at position %d => %ld\n",
                   found + 1, pos, permutations[found][pos]);
        }
        return permutations[found][pos];
    }
    if (opts.debug) {
        printf("map_groups(%d, %d, ..., %d, ...) next group is %d wide, needs %d\n",
               pos, found, group_total, grouping[found], needed[found]);
    }
    if (found == group_total) {
        /* Make sure only . or ? left */
        for (int i = pos; i < strlen(rec); i++) {
            if (rec[i] == '#') {
                return 0;
            }
        }
        if (opts.debug) {
            printf("Permutation found\n");
        }
        permutations[found][pos] = 1;
        return 1;
    }
    /* As long as there is enough room */
    while (strlen(rec) - pos >= needed[found]) {
        int width = grouping[found];
        /* Are the next X characters either '?' or '#' */
        bool possible = true;
        for (int i = 0; i < width; i++) {
            if (rec[pos + i] == '.') {
                possible = false;
                break;
            }
        }
        if (possible && rec[pos + width] != '#') {
            /* ?, ., or \0 */
            perm += map_groups(pos + width + 1, found + 1, rec);
        }

        if (rec[pos] == '#') {
            break;
        }

        /* ? or . */
        pos++;
    }
    permutations[found][orig_pos] = perm;
    return perm;
}


void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    long perm_total = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        char groups[MAX_LEN + 1];

        sscanf(buf, "%[?.#] %[0-9,]", record, groups);
        if (opts.debug) {
            printf("Record '%s' and groups '%s'\n", record, groups);
        }
        group_total = 0;
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
        /*
         * Unfold
         */
        for (int i = 1; i <= 5; i++) {
            for (int j = 0; j < group_total; j++) {
                grouping[group_total * i + j] = grouping[j];
            }
        }
        char new_rec[MAX_LEN + 1];
        strcpy(new_rec, record);
        for (int i = 0; i < 4; i++) {
            strcat(new_rec, "?");
            strcat(new_rec, record);
        }
        strcpy(record, new_rec);
        group_total *= 5;

        /*
         * Calculate space needed for each group, for some optimization
         */
        needed[group_total - 1] = grouping[group_total - 1];
        for (int i = group_total - 2; i >= 0; i--) {
            needed[i] = needed[i + 1] + 1 + grouping[i];
        }

        if (opts.debug) {
            printf("New record:\n    %s\n", record);
            printf("New group total:    %d\n", group_total);
            printf("Groups and needed arrays:\n");
            for (int i = 0; i < group_total; i++) {
                printf("    %3d - %3d\n", grouping[i], needed[i]);
            }
        }

        /*
         * Prefill the 'permutations' array (a bit overkill, but meh)
         */
        for (int i = 0; i <= group_total; i++) {
            for (int j = 0; j < MAX_LEN + 1; j++) {
                permutations[i][j] = -1;
            }
        }
        long perm = map_groups(0, 0, record);
        if (opts.debug) {
            printf("#Arrangements for %s: %ld\n", record, perm);
        }
        perm_total += perm;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Total #arrangements for this set: %ld\n", perm_total);
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
     %s - Program for AoC 2023 puzzles; Day 12, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 12, part 2.\n\
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
