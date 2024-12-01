#include "package.h"
#include "strings.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serial.h"
#include "termios.h"

// turn the serial connection into a raw serial connection
void make_raw(struct termios *tty) {
  cfmakeraw(tty);
  cfsetspeed(tty, B115200);

  tty->c_cflag &= ~PARENB; // disable parity and only one stop bit
  tty->c_cflag &= ~CSTOPB; // disable parity and only one stop bit

  tty->c_cc[VMIN] = 0;   // block until either amount of data is available
  tty->c_cc[VTIME] = 10; // wait for 1 second on read
}

// returns a file descriptor of the serial port
int init_serial(char *port_name) {
  struct termios tty; // communication settings

  int serial_port = open(port_name, O_RDWR);

  if (serial_port < 0) {
    printf("Error %d while opening serial port: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    close(serial_port);
    exit(EXIT_FAILURE);
  }

  make_raw(&tty);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    close(serial_port);
    exit(EXIT_FAILURE);
  }
  return serial_port;
}

// writes the Package `p` to the given `serial_port`
void write_package(int serial_port, struct Package *p) {
  uint8_t *package_bytes = to_byte_array(p);
  size_t array_len = get_array_len(p);

#ifdef DBG
  printf("Writing: ");
  for (int i = 0; i < array_len; i++) {
    printf("%02X", package_bytes[i]);
  }
  printf("\n");
#endif /* ifdef DBG */

  int res = write(serial_port, package_bytes, array_len);
  if (res <= 0) {
    printf("Write returned %d: %s\n", errno, strerror(errno));
  }

  free(package_bytes);
}

// Receives a package sent by the Dobot via the given port.
// The Package struct's params field must be deallocated manually.
struct Package receive_package(int serial_port) {
  // WARN: Maybe 256 bytes wont be enought at some point
  char *buf[256] = {0};
  int read_bytes = read(serial_port, &buf, sizeof(buf));

  if (read_bytes == 0) {
    printf("Malformed package was sent: No bytes received from "
           "Dobot.");
    exit(1);
  }

  struct Package p = from_byte_array((uint8_t *)&buf);

  return p;
}

// Receives a Package struct sent via the given file descriptor
//
// (`serial_port`) and evaluates its params as a string which is then returned.
//
// The returned string must be deallocated manually.
char *receive_string(int serial_port) {
  struct Package p = receive_package(serial_port);

  char *data = (char *)malloc(p.len - 1);
  memcpy(data, p.params, p.len - 2);
  data[p.len - 2] = 0;
  free_package(&p);

  return data;
}

