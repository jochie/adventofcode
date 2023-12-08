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


struct hand {
    char hand[6];
    int bid;
    int score;
};

char order[] = "AKQT98765432J";

/*
 * Should return a value from 0..12
 */
int
score_card(char card)
{
    char *info = strchr(order, card);
    return strlen(order) - (info - order) - 1;
}

/*
 * Helper function for sorting the cards based on their position in 'order'
 */
int
compare_chars(const void *a, const void *b)
{
    char c_a = *((char *)a);
    char c_b = *((char *)b);

    char *info1 = strchr(order, c_a);
    char *info2 = strchr(order, c_b);
    if (info1 == info2) {
        return 0;
    }
    if (info1 < info2) {
        return -1;
    }
    return 1;
}


/*
 * Should return a value of 0..13^5-1 (371292)
 */
int
score_cards(char *cards)
{
    int score = 0;

    for (int i = 0; i < 5; i++) {
        score = score * 13 + score_card(cards[i]);
    }
    return score;
}

char *
interpret_score(int score)
{
    if (score >= 9000000) {
        return "five of a kind";
    }
    if (score >= 8000000) {
        return "four of a kind";
    }
    if (score >= 7000000) {
        return "full house";
    }
    if (score >= 6000000) {
        return "three of a kind";
    }
    if (score >= 5000000) {
        return "two pairs";
    }
    if (score >= 4000000) {
        return "one pair";
    }
    return "high card";
}

int
hand_score(char *hand)
{
    int same_first, same_last, same_middle;
    char first, last, middle;
    char hand_ordered[6];
    int jokers = 0;

    for (int i = 0; i < 5; i++) {
        if (hand[i] == 'J') {
            jokers++;
        }
    }
    /* Get a sorted list of cards */
    strcpy(hand_ordered, hand);
    qsort(hand_ordered, 5, sizeof(char), compare_chars);

    first = hand_ordered[0];
    last  = hand_ordered[4];
    middle = hand_ordered[2];
    same_first = 1;
    for (int i = 1; i < 5; i++) {
        if (hand_ordered[i] == first) {
            same_first++;
        }
    }
    same_last = 1;
    for (int i = 3; i >= 0; i--) {
        if (hand_ordered[i] == last) {
            same_last++;
        }
    }
    same_middle = 0;
    for (int i = 0; i < 5; i++) {
        if (hand_ordered[i] == middle) {
            same_middle++;
        }
    }
    
    /* Five of a kind */
    if (same_first == 5) {
        return 9000000 + score_cards(hand);
    }
    /* Four of a kind */
    if (same_first == 4 || same_last == 4) {
        if (jokers == 1 || jokers == 4) {
            /* Five of a kind */
            return 9000000 + score_cards(hand);
        }
        return 8000000 + score_cards(hand);
    }
    /* Full house */
    if (same_first == 3 && same_last == 2) {
        if (jokers == 2 || jokers == 3) {
            /* Five of a kind */
            return 9000000 + score_cards(hand);
        }
        return 7000000 + score_cards(hand);
    }
    if (same_first == 2 && same_last == 3) {
        if (jokers == 2 || jokers == 3) {
            /* Five of a kind */
            return 9000000 + score_cards(hand);
        }
        return 7000000 + score_cards(hand);
    }
    /* Three of a kind */
    if (same_first == 3 || same_last == 3 || same_middle == 3) {
        if (jokers == 1 && middle != 'J') {
            /* Four of a kind */
            return 8000000 + score_cards(hand);
        }
        if (jokers == 3) {
            /* Four of a kind with one of the other cards */
            return 8000000 + score_cards(hand);
        }
        return 6000000 + score_cards(hand);
    }

    char pair_cards[2];
    int pairs = 0;
    for (int i = 0; i < strlen(order); i++) {
        char card = order[i];
        int found = 0;
        for (int j = 0; j < 5; j++) {
            if (hand[j] == card) {
                found++;
            }
        }
        if (found == 2) {
            pair_cards[pairs] = card;
            pairs++;
        }
    }
    /* Two pairs */
    if (pairs == 2) {
        if (pair_cards[0] == 'J' || pair_cards[1] == 'J') {
            /* Four of a kind, as one of the pairs is a pair of Jokers */
            return 8000000 + score_cards(hand);
        }
        if (jokers == 1) {
            /* Full house */
            return 7000000 + score_cards(hand);
        }
        return 5000000 + score_cards(hand);
    }
    /* One pair */
    if (pairs == 1) {
        if (pair_cards[0] == 'J') {
            /* Three of a kind, with some other random card */
            return 6000000 + score_cards(hand);
        }
        if (jokers == 0) {
            /* Still one pair */
            return 4000000 + score_cards(hand);
        }
        if (jokers == 1) {
            /* Three of a kind */
            return 6000000 + score_cards(hand);
        }
    }

    /* High card */
    if (jokers == 1) {
        /* One pair */
        return 4000000 + score_cards(hand);
    }
    return score_cards(hand);
}

/*
 * Compare such that the higher scores are at the beginning of the list
 */
int
compare_hands(const void *a, const void *b)
{
    struct hand *hand1 = (struct hand *)a;
    struct hand *hand2 = (struct hand *)b;

    if (hand1->score == hand2->score) {
        return 0;
    }
    if (hand1->score < hand2->score) {
        return -1;
    }
    return 1;
}



void
process_file(FILE *fd)
{
    char buf[MAX_LEN + 1];

    struct hand hands[1001];

    int total = 0;
    while (NULL != fgets(buf, MAX_LEN, fd)) {
        /* Strip the newline, if present */
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        if (opts.debug) {
            printf("DEBUG: Line received: '%s'\n", buf);
        }
        sscanf(buf, "%s %d", hands[total].hand, &hands[total].bid);

        /* Pre-calculate the score */
        hands[total].score = hand_score(hands[total].hand);
        if (opts.debug) {
            printf("Hand %4d is '%s', bid is %3d; Score = %d\n",
                   total + 1, hands[total].hand, hands[total].bid,
                   hands[total].score);
        }
        total++;
    }

    qsort(hands, total, sizeof(struct hand), compare_hands);

    int sum_bids = 0;
    for (int i = 0; i < total; i++) {
        if (opts.debug) {
            printf("Hand %4d is '%s', bid is %4d - SCORE: %7d - Type: %s\n",
                   i + 1, hands[i].hand, hands[i].bid,
                   hands[i].score, interpret_score(hands[i].score));
        }
        sum_bids += (i + 1) * hands[i].bid;
    }
    if (opts.debug) {
        printf("DEBUG: End of file\n");
    }
    printf("Sum of the bids * ranks: %d\n", sum_bids);
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
     %s - Program for AoC 2023 puzzles; Day 7, part 2\n\
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
     This program is used for one of the AoC 2023 puzzles; Day 7, part 2.\n\
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
