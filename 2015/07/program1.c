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

# define YEAR 2015
# define DAY     7
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

# define MAX_GATE 350
# define MAX_WIRE 350
# define MAX_TARGETS 10

typedef enum operator {
    ASSIGN,
    AND,
    OR,
    LSHIFT,
    RSHIFT,
    NOT
} operator;

char operator_names[][8] = {
    "ASSIGN",
    "AND",
    "OR",
    "LSHIFT",
    "RSHIFT",
    "NOT"
};

typedef struct wire {
    char id[3];
    int  value;
    int  source; /* gate reference */
    int  targets[MAX_TARGETS]; /* which gates depend on this wire */
    int  t_targets;
} wire;

typedef struct gate {
    int  lhs;
    int  lhs_wire;

    operator op;

    int  rhs;
    int  rhs_wire;
    int  value;

    int  dest_wire;
} gate;

struct circuit {
    gate gates[MAX_GATE];
    int t_gate;

    wire wires[MAX_WIRE];
    int t_wire;
} circuit;

void
circuit_init()
{
    circuit.t_gate = 0;
    circuit.t_wire = 0;
}

/* Find or create a wire for an identifier */
int
circuit_wire(char id[3])
{
    for (int i = 0; i < circuit.t_wire; i++) {
        if (!strcmp(circuit.wires[i].id, id)) {
            return i;
        }
    }
    if (MAX_WIRE == circuit.t_wire) {
        printf("Need more room for wires. Adjust MAX_WIRE, recompile, and try again.\n");
        exit(1);
    }
    int index = circuit.t_wire;
    strcpy(circuit.wires[index].id, id);
    circuit.wires[index].value = -1;
    circuit.wires[index].source = -1;
    circuit.wires[index].t_targets = 0;
    circuit.t_wire++;

    return index;
}

void
circuit_wire_target(int wire, int gate)
{
    if (MAX_TARGETS == circuit.wires[wire].t_targets) {
        printf("Unable to add more targets for %d (to gate %d). Recompile, and try again.\n",
               wire, gate);
        exit(1);
    }
    circuit.wires[wire].targets[circuit.wires[wire].t_targets] = gate;
    circuit.wires[wire].t_targets++;
}

void
circuit_add_solo(int val, char id[3], operator op, char dest[3])
{
    if (MAX_GATE == circuit.t_gate) {
        printf("Need more room for gates. Adjust MAX_GATE, recompile, and try again.\n");
        exit(1);
    }
    int index = circuit.t_gate;
    circuit.gates[index].op = op;
    int dest_wire = circuit_wire(dest);
    if (-1 != circuit.wires[dest_wire].source) {
        printf("Multiple gates feed to '%s'?\n", dest);
        exit(1);
    }
    circuit.wires[dest_wire].source = index;
    circuit.gates[index].dest_wire = dest_wire;
    if (NULL == id) {
        circuit.gates[index].lhs = val;
        circuit.gates[index].lhs_wire = -1;
    } else {
        circuit.gates[index].lhs = -1;
        circuit.gates[index].lhs_wire = circuit_wire(id);
        circuit_wire_target(circuit.gates[index].lhs_wire, index);
    }
    circuit.gates[index].rhs = 0;
    circuit.gates[index].rhs_wire = -1;

    circuit.t_gate++;
}

void
circuit_add_duo(int lhs, char lhs_id[3], operator op, int rhs, char rhs_id[3], char dest[3])
{
    if (MAX_GATE == circuit.t_gate) {
        printf("Need more room for gates. Adjust MAX_GATE, recompile, and try again.\n");
        exit(1);
    }
    int index = circuit.t_gate;
    circuit.gates[index].op = op;
    int dest_wire = circuit_wire(dest);
    if (-1 != circuit.wires[dest_wire].source) {
        printf("Multiple gates feed to '%s'?\n", dest);
        exit(1);
    }
    circuit.wires[dest_wire].source = index;
    circuit.gates[index].dest_wire = dest_wire;

    if (NULL == lhs_id) {
        circuit.gates[index].lhs = lhs;
        circuit.gates[index].lhs_wire = -1;
    } else {
        circuit.gates[index].lhs = -1;
        circuit.gates[index].lhs_wire = circuit_wire(lhs_id);
        circuit_wire_target(circuit.gates[index].lhs_wire, index);
    }
    if (NULL == rhs_id) {
        circuit.gates[index].rhs = rhs;
        circuit.gates[index].rhs_wire = -1;
    } else {
        circuit.gates[index].rhs = -1;
        circuit.gates[index].rhs_wire = circuit_wire(rhs_id);
        circuit_wire_target(circuit.gates[index].rhs_wire, index);
    }
    circuit.t_gate++;
}

void
circuit_dump_gate(int i)
{
    switch (circuit.gates[i].op) {
    case ASSIGN:
        if (circuit.gates[i].lhs_wire == -1) {
            printf("[G%03d] %d -> %s (%d)\n",
                   i,
                   circuit.gates[i].lhs,
                   circuit.wires[circuit.gates[i].dest_wire].id,
                   circuit.wires[circuit.gates[i].dest_wire].value);
        } else {
            printf("[G%03d] %s (%d) -> %s (%d)\n",
                   i,
                   circuit.wires[circuit.gates[i].lhs_wire].id,
                   circuit.wires[circuit.gates[i].lhs_wire].value,
                   circuit.wires[circuit.gates[i].dest_wire].id,
                   circuit.wires[circuit.gates[i].dest_wire].value);
        }
        break;
    case NOT:
        if (circuit.gates[i].lhs_wire == -1) {
            printf("[G%03d] NOT %d -> %s (%d)\n",
                   i,
                   circuit.gates[i].lhs,
                   circuit.wires[circuit.gates[i].dest_wire].id,
                   circuit.wires[circuit.gates[i].dest_wire].value);
        } else {
            printf("[G%03d] NOT %s (%d) -> %s (%d)\n",
                   i,
                   circuit.wires[circuit.gates[i].lhs_wire].id,
                   circuit.wires[circuit.gates[i].lhs_wire].value,
                   circuit.wires[circuit.gates[i].dest_wire].id,
                   circuit.wires[circuit.gates[i].dest_wire].value);
        }
        break;
    case AND:
    case OR:
    case LSHIFT:
    case RSHIFT:
        if (circuit.gates[i].lhs_wire == -1) {
            if (circuit.gates[i].rhs_wire == -1) {
                printf("[G%03d] %d %s %d -> %s (%d)\n",
                       i,
                       circuit.gates[i].lhs,
                       operator_names[circuit.gates[i].op],
                       circuit.gates[i].rhs,
                       circuit.wires[circuit.gates[i].dest_wire].id,
                       circuit.wires[circuit.gates[i].dest_wire].value);
            } else {
                printf("[G%03d] %d %s %s (%d) -> %s (%d)\n",
                       i,
                       circuit.gates[i].lhs,
                       operator_names[circuit.gates[i].op],
                       circuit.wires[circuit.gates[i].rhs_wire].id,
                       circuit.wires[circuit.gates[i].rhs_wire].value,
                       circuit.wires[circuit.gates[i].dest_wire].id,
                       circuit.wires[circuit.gates[i].dest_wire].value);
            }
        } else {
            if (circuit.gates[i].rhs_wire == -1) {
                printf("[G%03d] %s (%d) %s %d -> %s (%d)\n",
                       i,
                       circuit.wires[circuit.gates[i].lhs_wire].id,
                       circuit.wires[circuit.gates[i].lhs_wire].value,
                       operator_names[circuit.gates[i].op],
                       circuit.gates[i].rhs,
                       circuit.wires[circuit.gates[i].dest_wire].id,
                       circuit.wires[circuit.gates[i].dest_wire].value);
            } else {
                printf("[G%03d] %s (%d) %s %s (%d) -> %s (%d)\n",
                       i,
                       circuit.wires[circuit.gates[i].lhs_wire].id,
                       circuit.wires[circuit.gates[i].lhs_wire].value,
                       operator_names[circuit.gates[i].op],
                       circuit.wires[circuit.gates[i].rhs_wire].id,
                       circuit.wires[circuit.gates[i].rhs_wire].value,
                       circuit.wires[circuit.gates[i].dest_wire].id,
                       circuit.wires[circuit.gates[i].dest_wire].value);
            }
        }
        break;
    default:
        printf("What? Unexpected operator value at %d: %d\n",
               i, circuit.gates[i].op);
        exit(1);
        break;
    }
}

void
circuit_dump()
{
    printf("Wires:\n");
    for (int i = 0; i < circuit.t_wire; i++) {
        printf("[W%03d] %s: %d\n", i, circuit.wires[i].id, circuit.wires[i].value);
        printf("     Source:  G%03d\n", circuit.wires[i].source);
        printf("     Targets:");
        for (int j = 0; j < circuit.wires[i].t_targets; j++) {
            printf(" G%03d", circuit.wires[i].targets[j]);
        }
        printf("\n");
    }
    printf("Gates:\n");
    for (int i = 0; i < circuit.t_gate; i++) {
        circuit_dump_gate(i);
    }
}

int
circuit_gate_eval(int gate)
{
    int lhs, rhs;

    if (-1 == circuit.gates[gate].lhs_wire) {
        lhs = circuit.gates[gate].lhs;
    } else {
        lhs = circuit.wires[circuit.gates[gate].lhs_wire].value;
        if (lhs == -1) {
            /* printf("Waiting for a value of LHS (%d) of gate %d\n", circuit.gates[gate].lhs_wire, gate); */
            return -1;
        }
    }
    if (-1 == circuit.gates[gate].rhs_wire) {
        rhs = circuit.gates[gate].rhs;
    } else {
        rhs = circuit.wires[circuit.gates[gate].rhs_wire].value;
        if (rhs == -1) {
            /* printf("Waiting for a value of RHS (%d) of gate %d\n", circuit.gates[gate].rhs_wire, gate); */
            return -1;
        }
    }
    int dest = circuit.gates[gate].dest_wire;
    switch (circuit.gates[gate].op) {
    case ASSIGN:
        circuit.wires[dest].value = lhs;
        break;
    case NOT:
        circuit.wires[dest].value = 0xffff & ~lhs;
        break;
    case AND:
        circuit.wires[dest].value = lhs & rhs;
        break;
    case OR:
        circuit.wires[dest].value = lhs | rhs;
        break;
    case LSHIFT:
        circuit.wires[dest].value = 0xffff & (lhs << rhs);
        break;
    case RSHIFT:
        circuit.wires[dest].value = lhs >> rhs;
        break;
    default:
        printf("Unexpected operand: %d\n",
               circuit.gates[gate].op);
        exit(1);
        break;
    }
    return circuit.wires[dest].value;
}

# define MAX_QUEUE 350

int queue[MAX_QUEUE];
int t_queue;

void
queue_init()
{
    t_queue = 0;
}

void
queue_add(int w)
{
    if (MAX_QUEUE == t_queue) {
        printf("Increase the queue size (MAX_QUEUE), recompile, and try again.\n");
        exit(1);
    }
    for (int i = 0; i < t_queue; i++) {
        if (queue[i] == w) {
            /* Already in the queue */
            return;
        }
    }
    queue[t_queue] = w;
    t_queue++;
}

void
circuit_eval()
{
    queue_init();

    for (int i = 0; i < circuit.t_gate; i++) {
        if (circuit.gates[i].op == ASSIGN && circuit.gates[i].lhs_wire == -1) {
            /* Start with the constant assignments */
            queue_add(i);
        }
    }
    while (t_queue > 0) {
        int copy[MAX_QUEUE];
        int t_copy = t_queue;
        memcpy(copy, queue, sizeof(queue));
        queue_init();

        for (int i = 0; i < t_copy; i++) {
            if (opts.debug) {
                printf("Attempting to evaluate G%03d...\n", copy[i]);
            }
            int result = circuit_gate_eval(copy[i]);
            if (result == -1) {
                /* We'll see it again when the other source of this
                   gate gets an actual value */
                if (opts.debug) {
                    printf("G%03d is not ready to be evaluated, yet.\n", copy[i]);
                    circuit_dump_gate(copy[i]);
                }
            } else {
                if (opts.debug) {
                    printf("Evaluated gate G%03d to %d (wire %s).\n",
                           copy[i], result,
                           circuit.wires[circuit.gates[copy[i]].dest_wire].id);
                }
                if (opts.verbose || opts.debug) {
                    circuit_dump_gate(copy[i]);
                }
                int dest = circuit.gates[copy[i]].dest_wire;
                if (!strcmp("a", circuit.wires[dest].id)) {
                    /* We're done! Won't happen for the sample circuit
                       obviously */
                    printf("Found a value for the 'a' wire: %d\n", result);
                    return;
                }
                for (int j = 0; j < circuit.wires[dest].t_targets; j++) {
                    queue_add(circuit.wires[dest].targets[j]);
                }
            }
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

    circuit_init();

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
        char id1[3], id2[3], id3[3];
        int val;

        if        (sscanf(buf, "%[a-z] AND %[a-z] -> %[a-z]", id1, id2, id3) == 3) {
            circuit_add_duo(0, id1, AND, 0, id2, id3);

        } else if (sscanf(buf, "%d AND %[a-z] -> %[a-z]", &val, id1, id3) == 3) {
            circuit_add_duo(val, NULL, AND, 0, id1, id3);

        } else if (sscanf(buf, "%[a-z] OR %[a-z] -> %[a-z]", id1, id2, id3) == 3) {
            circuit_add_duo(0, id1, OR, 0, id2, id3);

        } else if (sscanf(buf, "%[a-z] LSHIFT %d -> %[a-z]", id1, &val, id3) == 3) {
            circuit_add_duo(0, id1, LSHIFT, val, NULL, id3);

        } else if (sscanf(buf, "%[a-z] RSHIFT %d -> %[a-z]", id1, &val, id3) == 3) {
            circuit_add_duo(0, id1, RSHIFT, val, NULL, id3);

        } else if (sscanf(buf, "NOT %[a-z] -> %[a-z]", id1, id3) == 2) {
            circuit_add_solo(0, id1, NOT, id3);

        } else if (sscanf(buf, "%d -> %[a-z]", &val, id3) == 2) {
            circuit_add_solo(val, NULL, ASSIGN, id3);

        } else if (sscanf(buf, "%[a-z] -> %[a-z]", id1, id3) == 2) {
            circuit_add_solo(0, id1, ASSIGN, id3);

        } else {
            printf("Failed to parse line: '%s'\n", buf);
            exit(1);
        }
    }
    if (opts.debug) {
        circuit_dump();
    }

    circuit_eval();

    if (opts.debug) {
        circuit_dump();
    }
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
