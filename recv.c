#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "utils.h"
#include "queue.h"

#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv) {
  pkg package;
  char* filename;
  int fd = -1, i;
  int window_size = MIN_WINDOW;
  int file_size = MAX_FILESIZE;
  int delay = MAX_DELAY;
  int package_no = MAX_FILESIZE / MAX_PAYLOAD;
  q_head* buffer = new_queue();

  init(HOST,PORT);
  
  while (1) {
    if (buffer->len == package_no + 1) {
      break;
    }
    if (recv_pkg_timeout(&package, delay) < 0) {
      // If timed out, send a window of NACKs
      // for the first chunk of consecutive missing
      // frames. The sender (send.c) will handle these
      // nack'd packages and send them accordingly, keeping
      // the connection as busy as possible.
      package.type = NACK;
      
      int size;
      int* missing_idxs = get_window_of_missing(buffer, &size);

      if (missing_idxs != NULL) {
        int count = 0;
        // If a window of unreceived sequential frames exists
        for (i = 0; (i < size) && (count < window_size); ++i) {
          package.idx = missing_idxs[i];
          count++;
          
          send_pkg(&package);
        }
      } else {
        // Sent NACK for next missing frame
        package.idx = get_first_missing_pkg(buffer);
        send_pkg(&package);
      }
      continue;
    }
    if (!valid_cs(&package)) {
      // If received pkg is corrupted, request
      // the next missing frame in order
      package.type = NACK;
      package.idx = get_first_missing_pkg(buffer);

      send_pkg(&package);
    } else {
      if (package.type == INIT) {
        // Manage output file name:
        filename = malloc(6 + strlen((char*) (package.payload + 3 * sizeof(int))));
        sprintf(filename, "recv_%s", (char*) (package.payload + 3 * sizeof(int)));

        // Save file size:
        memcpy(&file_size, package.payload, sizeof(int));

        // Save delay:
        memcpy(&delay, package.payload + sizeof(int), sizeof(int));

        // Save window size:
        memcpy(&window_size, package.payload + 2 * sizeof(int), sizeof(int));

        // Compute no. of total frames (excepting INIT pkg)
        package_no = compute_no_of_frames(file_size);

        // Count it as valid and successfully received
        push(buffer, package);
        send_ack(&package);
      } else {
        // Save frame as it is valid
        push(buffer, package);
        send_ack(&package);

        // If next expected frame hasn't arrived
        // for WINDOW_SIZE frames, request it
        int tmp = get_first_missing_pkg(buffer);
        if (package.idx - tmp > window_size) {
          package.idx = tmp;
          send_pkg(&package);
        }
      }
    }
  }
  // Pop INIT package and open file
  pop(buffer);
  fd = open(filename, O_WRONLY | O_CREAT, 0644);

  // Flush buffer to file
  while (!is_empty(buffer)) {
    package = pop(buffer);
    write(fd, package.payload, package.payload_len);
  }

  // Send terminating signal to sender
  package.type = _EOF;
  send_pkg(&package);

  close(fd);
  free(filename);
  destroy_queue(buffer);

  return 0;
}
