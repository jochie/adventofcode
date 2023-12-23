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
# define DAY    20
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

# define LOW false
# define HIGH true

typedef enum type {
    START,
    FLIP,
    CONJ
} type;

typedef struct module {
    type t;
    char name[20];
    char targets[20][20];
    int t_total;
    struct inputs {
        bool state;
        int mod;
    } inputs[20];
    int i_total;
    bool state;
} module;

module modules[100];
int count = 0;

/*
 * Key sentence:
 *
 * "If a flip-flop module receives a high pulse, it is ignored and nothing happens."
 */

void
dump_module(int i)
{
    switch (modules[i].t) {
    case START:
        printf("Starter module (%s):\n", modules[i].name);
        break;
    case FLIP:
        printf("Flip-flop module (%s):\n", modules[i].name);
        printf("  Current state: %s\n", modules[i].state ? "-high" : "-low");
        break;
    case CONJ:
        printf("Conjunctor module (%s):\n", modules[i].name);
        printf("  Current state: %s\n", modules[i].state ? "-high" : "-low");
        break;
    }
    printf("  Targets (%d):\n    ", modules[i].t_total);
    for (int j = 0; j < modules[i].t_total; j++) {
        printf("%s ", modules[i].targets[j]);
    }
    printf("\n");
    if (modules[i].i_total > 0) {
        printf("  Inputs (%d):\n", modules[i].i_total);
        for (int j = 0; j < modules[i].i_total; j++) {
            printf("    %d (%s) -> %s\n",
                   modules[i].inputs[j].mod,
                   modules[modules[i].inputs[j].mod].name,
                   modules[i].inputs[j].state ? "-high" : "-low");
        }
    }
    printf("\n");
}

int high_count = 0, low_count = 0;
int broadcaster = 0;

struct queue {
    char oname[20];
    int origin;
    int module;
    bool value;
} queue[100], queue_copy[100];

int queue_size = 0;

int
find_module(char name[20])
{
    for (int i = 0; i < count; i++) {
        if (!strcmp(modules[i].name, name)) {
            return i;
        }
    }
    /* Not found, like "output"? */
    return -1;
}

void
queue_pulse(char name[20], int m, bool v)
{
    int o = find_module(name);

    if (opts.debug) {
        printf("%s (%d) %s-> %s\n",
               name, o, v ? "-high" : "-low", modules[m].name);
    }
    if (v) {
        high_count++;
    } else {
        low_count++;
    }
    strcpy(queue[queue_size].oname, name);
    queue[queue_size].origin = o;
    queue[queue_size].module = m;
    queue[queue_size].value = v;
    queue_size++;
}

void
find_module_inputs()
{
    for (int m = 0; m < count; m++) {
        for (int t = 0; t < modules[m].t_total; t++) {
            int i = find_module(modules[m].targets[t]);
            if (i == -1) {
                continue;
            }
            if (modules[i].t == CONJ) {
                if (opts.debug) {
                    printf("Connecting target %s of %s to input of %s\n",
                           modules[m].targets[t], modules[m].name, modules[i].name);
                }
                modules[i].inputs[modules[i].i_total].mod = m;
                modules[i].inputs[modules[i].i_total].state = LOW;
                modules[i].i_total++;
            }
        }
    }
}

void
process_queue()
{
    if (opts.debug) {
        printf("QUEUE size: %d\n", queue_size);
    }
    int copy_size = queue_size;
    for (int i = 0; i < copy_size; i++) {
        strcpy(queue_copy[i].oname, queue[i].oname);
        queue_copy[i].origin = queue[i].origin;
        queue_copy[i].module = queue[i].module;
        queue_copy[i].value = queue[i].value;
    }
    queue_size = 0;
    for (int i = 0; i < copy_size; i++) {
        int ori = queue_copy[i].origin;
        int mod = queue_copy[i].module;
        bool val = queue_copy[i].value;

        if (mod == -1) {
            if (opts.debug) {
                printf("Sent to a module without inputs or outputs.\n");
            }
            continue;
        }
        switch (modules[mod].t) {
        case START:
            for (int m = 0; m < modules[mod].t_total; m++) {
                int target = find_module(modules[mod].targets[m]);
                queue_pulse(modules[mod].name, target, val);
            }
            break;
        case FLIP:
            if (queue_copy[i].value == HIGH) {
                if (opts.debug) {
                    printf("Flip-flop module %s received a high pulse, nothing happens.\n",
                           modules[mod].name);
                }
                break;
            }
            modules[mod].state = !modules[mod].state;
            if (opts.debug) {
                printf("Flip-flop module %s received a low pulse and flipped to %d\n",
                       modules[mod].name, modules[mod].state);
            }
            for (int m = 0; m < modules[mod].t_total; m++) {
                int target = find_module(modules[mod].targets[m]);
                queue_pulse(modules[mod].name, target, modules[mod].state);
            }
            break;
        case CONJ:
            {
                bool all_high = true;
                for (int m = 0; m < modules[mod].i_total; m++) {
                    if (modules[mod].inputs[m].mod == ori) {
                        if (opts.debug) {
                            printf("Updating memory of input %d (%s) for %d (%s) to %d.\n",
                                   ori, modules[ori].name, mod, modules[mod].name, val);
                        }
                        modules[mod].inputs[m].state = val;
                    }
                    if (!modules[mod].inputs[m].state) {
                        all_high = false;
                    }
                }
                if (opts.debug) {
                    printf("All-high check for %s: %d\n", modules[mod].name, all_high);
                }
                all_high = !all_high;
                for (int m = 0; m < modules[mod].t_total; m++) {
                    int target = find_module(modules[mod].targets[m]);
                    queue_pulse(modules[mod].name, target, all_high);
                }
            }
            break;
        }
    }
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
        char *info = buf;

        if (buf[0] == '&') {
            modules[count].t = CONJ;
            info = buf + 1;
            modules[count].state = false;
        } else if (buf[0] == '%') {
            modules[count].t = FLIP;
            modules[count].state = false;
            info = buf + 1;
        } else {
            modules[count].t = START;
            broadcaster = count;
            info = buf;
        }
        int len;
        sscanf(info, "%s -> %n", modules[count].name, &len);
        info = info + len;

        int total = 0;
        while (true) {
            sscanf(info, "%[a-z]%n", modules[count].targets[total], &len);
            total++;
            if (info[len] == ',') {
                info = info + len + 2;
            } else {
                break;
            }
        }
        modules[count].t_total = total;
        modules[count].i_total = 0; /* post process for this */
        count++;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    find_module_inputs();
    if (opts.debug) {
        for (int m = 0; m < count; m++) {
            dump_module(m);
        }
        printf("Parsed %d modules.\n", count);
    }
    for (int pushes = 0; pushes < 1000; pushes++) {
        queue_pulse("button", broadcaster, LOW);

        int round = 0;
        while (queue_size > 0) {
            round++;
            if (opts.debug) {
                printf("\nROUND %d\n\n", round);
            }
            process_queue();
        }
    }
    if (opts.debug) {
        printf("\nDONE\n\n");
    }
    printf("%d [High pulses sent]) * %d [Low pulses sent] = %d\n", high_count, low_count, high_count * low_count);
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
