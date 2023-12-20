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
# define DAY    19
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

typedef enum names {
    NAME_X,
    NAME_M,
    NAME_A,
    NAME_S
} name;

typedef struct rule {
    name attr;
    char op;
    int  value;
    char target[5];
} rule;

typedef struct workflow {
    char name[5];
    rule rules[5];
    int  count;
    char fallback[5];
} workflow;

workflow workflows[600];

int count = 0;


bool
process_workflows(int flow, int attr[4])
{
    if (opts.debug) {
        printf("process_workflows(%d, %d, %d, %d)\n",
               attr[0], attr[1], attr[2], attr[3]);
    }
    while (true) {
        char *next = NULL;

        if (opts.debug) {
            printf("Checking against workflow %d\n", flow);
        }
        for (int r = 0; r < workflows[flow].count; r++) {
            if (opts.debug) {
                printf("  Checking against rule %d: %d %c %d (attr[%d] = %d)\n",
                       r,
                       workflows[flow].rules[r].attr,
                       workflows[flow].rules[r].op,
                       workflows[flow].rules[r].value,
                       workflows[flow].rules[r].attr,
                       attr[workflows[flow].rules[r].attr]);
            }
            switch (workflows[flow].rules[r].op) {
            case '<':
                if (attr[workflows[flow].rules[r].attr] < workflows[flow].rules[r].value) {
                    next = workflows[flow].rules[r].target;
                }
                break;
            case '>':
                if (attr[workflows[flow].rules[r].attr] > workflows[flow].rules[r].value) {
                    next = workflows[flow].rules[r].target;
                }
                break;
            default:
                printf("What operator is this? %c\n", workflows[flow].rules[r].op);
                exit(1);
            }
            if (NULL != next) {
                break; /* for */
            }
        }
        if (NULL == next) {
            next = workflows[flow].fallback;
        }
        if (!strcmp(next, "A")) {
            return true;
        }
        if (!strcmp(next, "R")) {
            return false;
        }
        for (int w = 0; w < count; w++) {
            if (!strcmp(workflows[w].name, next)) {
                if (opts.debug) {
                    printf("Moving on to %s (%d)\n", next, w);
                }
                flow = w;
                break;
            }
        }
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
    int total = 0;

    count = 0;

    /* Parse workflows */
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
        if (!strlen(buf)) {
            break;
        }
        char name[5];
        char info[MAX_LEN + 1];
        sscanf(buf, "%[a-z]{%[^}]}", name, info);

        strcpy(workflows[count].name, name);
        workflows[count].count = 0;

        int total = 0;
        char *p = info;
        while (true) {
            char *sep = strchr(p, ',');
            if (NULL == sep) {
                if (opts.debug) {
                    printf("  Fallback: %s\n", p);
                }
                strcpy(workflows[count].fallback, p);
                break;
            }
            sep[0] = '\0';
            char attr, op;
            int value;
            char target[5];
            sscanf(p, "%c%c%d:%s", &attr, &op, &value, target);
            switch (attr) {
            case 'x': workflows[count].rules[total].attr = NAME_X; break;
            case 'm': workflows[count].rules[total].attr = NAME_M; break;
            case 'a': workflows[count].rules[total].attr = NAME_A; break;
            case 's': workflows[count].rules[total].attr = NAME_S; break;
            }
            workflows[count].rules[total].op = op;
            workflows[count].rules[total].value = value;
            strcpy(workflows[count].rules[total].target, target);
            if (opts.debug) {
                printf("  Rule: %c %c %d => %s\n", attr, op, value, target);
            }
            total++;
            p = sep + 1;
        }
        workflows[count].count = total;
        if (opts.debug) {
            printf("Total of %d rules\n", total);
        }
        count++;
    }
    if (opts.debug) {
        printf("Found a total of %d workflows\n", count);
    }

    int flow_start = 0;
    for (int w = 0; w < count; w++) {
        if (!strcmp(workflows[w].name, "in")) {
            flow_start = w;
            break;
        }
    }
    if (opts.debug) {
        printf("Workflow to start at: %d\n", flow_start);
        printf("\n");
    }
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
        int attr[4];
        sscanf(buf, "{x=%d,m=%d,a=%d,s=%d}", &attr[0], &attr[1], &attr[2], &attr[3]);

        if (process_workflows(flow_start, attr)) {
            int sum = attr[0] + attr[1] + attr[2] + attr[3];
            if (opts.debug) {
                printf("  Accepted item: %d\n", sum);
            }
            total += sum;
        }
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Total accepted: %d\n", total);
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
