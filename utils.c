#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "link_emulator/lib.h"
#include "utils.h"

void check_err(char* msg, int err) {
    if (err < 0) {
        perror(msg);
        exit(1);
    }
}

void send_pkg(pkg* package) {
    msg t;

    memcpy(&(t.payload), package, MSGSIZE);
    t.len = package->payload_len;

    check_err("Error sending message!", send_message(&t));
}

void recv_pkg(pkg* package) {
    msg r;

    check_err("Error receiving message!", recv_message(&r));

    memset(package, 0, sizeof(pkg));
    *package = *((pkg *) r.payload);
}

int recv_pkg_timeout(pkg* package, int delay) {
    msg r;
    const int TIME_OUT = delay;

    if (recv_message_timeout(&r, TIME_OUT) < 0) {
        return -1;
    }

    memset(package, 0, sizeof(pkg));
    *package = *((pkg *) r.payload);
    return 1;
}

void compute_checksum(pkg* package) {
    char cs = package->type;
    int i;
    
    for (i = 0; i < package->payload_len; ++i) {
        cs ^= package->payload[i];
    }

	for(i = 0; i < sizeof(int); ++i) {
        char* byte1 = (char*) &package->idx + i;
        char* byte2 = (char*) &package->payload_len + i;

		cs ^= *byte1;
        cs ^= *byte2;
	}

    package->checksum = cs;
}

int valid_cs(pkg* package) {
    char cs = package->checksum;

    compute_checksum(package);

    return (package->checksum == cs) ? 1 : 0;
}

void send_ack(pkg* package) {
    pkg ack;

    ack.idx = package->idx;
    ack.type = ACK;

    send_pkg(&ack);
}

int compute_no_of_frames(int file_size) {
    return (file_size % MAX_PAYLOAD == 0) ?
            (file_size / MAX_PAYLOAD) : (file_size / MAX_PAYLOAD) + 1;
}