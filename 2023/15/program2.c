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
# define DAY    15
# define PART    2

# define STR(x) _STR(x)
# define _STR(x) #x

# define MAX_LEN 32768

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


/*
 * Data structure to keep my head sane, with ample buffer for however
 * many lenses might theoretically end up in one of the boxes, and
 * crazy extra space for the labels, much more than we saw in practice
 * in the input.
 */
struct box {
    int total;
    struct lens {
        char label[100];
        int focal;
    } lenses[100];
} boxes[256];


/*
 * If a lens with the same label is already present, put this one in
 * its place. Otherwise add an entry at the end of the list.
 */
void
add_to_box(int box, char *label, int focal)
{
    int total = boxes[box].total;

    if (total == 0) {
        strcpy(boxes[box].lenses[0].label, label);
        boxes[box].lenses[0].focal = focal;
        boxes[box].total = 1;
        return;
    }
    for (int i = 0; i < boxes[box].total; i++) {
        if (!strcmp(boxes[box].lenses[i].label, label)) {
            /* Replace this lens */
            boxes[box].lenses[i].focal = focal;
            return;
        }
    }
    strcpy(boxes[box].lenses[total].label, label);
    boxes[box].lenses[total].focal = focal;
    boxes[box].total++;
}


/*
 * If a lens with the same label is found, remove it, and shift the
 * other lenses forward
 */
void
remove_from_box(int box, char *label)
{
    for (int i = 0; i < boxes[box].total; i++) {
        if (!strcmp(boxes[box].lenses[i].label, label)) {
            for (int j = i; j < boxes[box].total; j++) {
                strcpy(boxes[box].lenses[j].label, boxes[box].lenses[j + 1].label);
                boxes[box].lenses[j].focal = boxes[box].lenses[j + 1].focal;
            }
            boxes[box].total--;
            return;
        }
    }
}

/*
 * Process one of the lens placement instructions
 */
void
process_step(char *step)
{
    char label[100];

    int val = 0;
    int i = 0;
    for (; i < strlen(step); i++) {
        if (step[i] >= 'a' && step[i] <= 'z') {
            label[i] = step[i];
            val = ((val + step[i]) * 17) % 256;
        } else {
            /* Collect label characters as we go. */
            label[i] = '\0';
            break;
        }
    }

    int foc_len;

    if (opts.debug) {
        printf("Label %s value: %d\n", label, val);
    }
    switch (step[i]) {
    case '=':
        foc_len = step[i + 1] - '0';
        if (opts.debug) {
            printf("focal length for box %d: %d\n", val, foc_len);
        }
        add_to_box(val, label, foc_len);
        break;
    case '-':
        if (opts.debug) {
            printf("Remove lens from box %d?\n", val);
        }
        remove_from_box(val, label);
        break;
    }
    if (opts.debug) {
        for (int i = 0; i < 256; i++) {
            if (boxes[i].total > 0) {
                printf("Box %d:", i);
                for (int j = 0; j < boxes[i].total; j++) {
                    printf(" [%s %d]", boxes[i].lenses[j].label, boxes[i].lenses[j].focal);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}


/*
 * Calculate the focal power of the lenses in one of the boxes
 */
int
box_power(int box)
{
    int power = 0;

    for (int i = 0; i < boxes[box].total; i++) {
        power += (box + 1) * (i + 1) * boxes[box].lenses[i].focal;
    }
    return power;
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

    for (int i = 0; i < 256; i++) {
        boxes[i].total = 0;
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
        char *cur = buf;
        while (true) {
            char *sep;

            sep = strchr(cur, ',');
            if (NULL != sep) {
                sep[0] = '\0';
            }
            if (opts.debug) {
                printf("Step: %s\n", cur);
            }
            process_step(cur);
            if (NULL == sep) {
                break;
            }
            cur = sep + 1;
        }
    }
    int sum_power = 0;
    for (int i = 0; i < 256; i++) {
        if (boxes[i].total > 0) {
            int power = box_power(i);
            if (opts.debug) {
                printf("Box %d power: %d\n", i, power);
            }
            sum_power += power;
        }
    }
    printf("Sum of power: %d\n", sum_power);
    if (opts.debug) {
        printf("DEBUG: End of file\n");
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
