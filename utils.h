#include <stdlib.h>
#include "link_emulator/lib.h"

#ifndef _UTILS
#define _UTILS

// Can't define enum. Size is crucial
#define INIT 0
#define DATA 1
#define ACK 2
#define NACK 3
#define _EOF 4

#define MIN_WINDOW 7
#define EPSILON 10
#define MAX_FILESIZE (10 * (1 << 20) + EPSILON)  // 10MB + 10B
#define MAX_DELAY 1000
#define MAX_PAYLOAD 1390
#define MIN(a, b) ((a < b) ? a : b)

typedef struct _pkg {
    int idx;
    int payload_len;
    char checksum;
    char type;
    char payload[MAX_PAYLOAD];
} pkg;

/*
    Exits the program with error code 1
    and prints message to stderr if second
    param is an error code.
*/
void check_err(char* msg, int err_code);

/*
    Checks for error code, manages the cast
    and sends frame.
*/
void send_pkg(pkg* package);

/*
    A wrapper function for recv_message
    that casts and returns the frame
    as a "pkg" and verifies the error code.
*/
void recv_pkg(pkg* package);

/*
    A wrapper function for recv_message_timeout.
    Casts package into a "pkg" data type.
*/
int recv_pkg_timeout(pkg* package, int delay);

/*
    Checks validity of the checksum.
    Returns 1 if they match, 0 otherwise.
*/
int valid_cs(pkg* package);

/*
    Computes the checksum of the whole
    package.
*/
void compute_checksum(pkg* package);

/*
    Sends ack with package's idx.
*/
void send_ack(pkg* package);

/*
    Computes the total number of frames
    needed for a file of file_size.
*/
int compute_no_of_frames(int file_size);

#endif