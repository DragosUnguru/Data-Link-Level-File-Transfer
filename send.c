#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "utils.h"
#include "queue.h"

#define HOST "127.0.0.1"
#define PORT 10000

void send_window(q_head* buffer, int WINDOW_SIZE) {
  int count = 0;
  pkg package;
  queue* tmp = buffer->front;

  while ((count < WINDOW_SIZE) && (tmp != NULL)) {
    package = tmp->package;
    count++;
    tmp = tmp->next;

    send_pkg(&package);
  }
}

int main(int argc,char** argv) {
  const char* FILENAME = argv[1];
  const int SPEED = atoi(argv[2]);
	const int DELAY = atoi(argv[3]);
  const int WINDOW_SIZE = (SPEED * DELAY * 1000) / (sizeof(msg) * 8);

  pkg package;
  int fd, file_size;

  // Buffer represented as a queue
  q_head* buffer = new_queue();

  init(HOST,PORT);

  // Open file
  fd = open(FILENAME, O_RDONLY);
  check_err("Error opening file!", fd);

  // Compute file size and no. of packages
  file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  int package_no = compute_no_of_frames(file_size);

  // Initialize first package with delay, file name and size
  package.type = INIT;
  package.idx = 0;
  package.payload_len = 3 * sizeof(int) + strlen(FILENAME);
  memcpy(package.payload, &file_size, sizeof(int));
  memcpy(package.payload + sizeof(int), &DELAY, sizeof(int));
  memcpy(package.payload + 2 * sizeof(int), &WINDOW_SIZE, sizeof(int));
  memcpy(package.payload + 3 * sizeof(int), FILENAME, strlen(FILENAME) + 1);
  compute_checksum(&package);
  // Buffer package
  push(buffer, package);

  // Fill buffer with file data
  int i;
  for (i = 1; i <= package_no; ++i) {
    ssize_t err = read(fd, &(package.payload), MAX_PAYLOAD);
    check_err("Error reading file!", err);
    // Create package
    package.idx = i;
    package.type = DATA;
    package.payload_len = err;
    compute_checksum(&package);

    push(buffer, package);
  }

  // Flood wire with window_size pkgs
  int last_idx_sent;
  queue* next_unsent = buffer->front;
  for (last_idx_sent = 0; last_idx_sent < MIN(WINDOW_SIZE, package_no + 1); ++last_idx_sent) {
    package = next_unsent->package;
    next_unsent = next_unsent->next;

    send_pkg(&package);
  }

  while (1) {
    pkg ack;
    if (next_unsent == NULL) {
      next_unsent = buffer->front;
    }

    if (recv_pkg_timeout(&ack, DELAY) < 0) {
      // If connection timed out, the "wire"
      // is free. Resend last not-ackd window

      send_window(buffer, WINDOW_SIZE);
    } else {
      if (ack.type == ACK) {
        // Pop package from buffer as it was
        // successfully sent
        pop_idx(buffer, ack.idx);

        if (buffer->len > 1) {
          // Send next unsent-yet frame
          package = next_unsent->package;
          next_unsent = next_unsent->next;

          send_pkg(&package);
        }
      }
      else if (ack.type == NACK) {
        // Recv requested his next in order
        // unreceived pkg. Resend
        package = get_pkg(buffer, ack.idx);

        if (package.idx == -1) {
          package = next_unsent->package;
          next_unsent = next_unsent->next;
        }
        send_pkg(&package);
      }
      else if (ack.type == _EOF) {
        break;
      }
    }
  }

  destroy_queue(buffer);
  close(fd);

  return 0;
}
