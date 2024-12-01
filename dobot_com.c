#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

// The package struct used to communicate with the dobot
struct Package {
  uint8_t len;

  uint8_t id;
  uint8_t ctrl; // fist bit is rw, second bit is is_queued

  uint8_t* params;
};

// calculates the number of bytes that byte array needs
// in order for the package to to fit inside
size_t get_array_len(struct Package* p) {
  return (sizeof(uint8_t) * 2)  // Header bytes
         + sizeof(uint8_t)      // len
         + p->len               // payload length
         + sizeof(uint8_t);     // checksum
}

// calculates the checksum of a package
uint8_t calculate_checksum(struct Package *p) {
  uint8_t result = p->id + p->ctrl;

  // add up params
  uint8_t param_len = p->len - 2;
  for (size_t i = 0; i < param_len; i++) {
    result += p->params[i];
  }

  return (256 - result);
}

// allocates memory to copy the package into a continuos array of bytes
uint8_t* to_byte_array(struct Package* p) {
  uint8_t* bytes = (uint8_t*) malloc(get_array_len(p));

  // set header bytes in array
  memset(bytes, 0xAA, sizeof(uint8_t) * 2);

  memcpy(bytes + 2, &p->len, sizeof(uint8_t));

  // cpy payload into array
  memcpy(bytes + 3, &p->id, sizeof(uint8_t));
  memcpy(bytes + 4, &p->ctrl, sizeof(uint8_t));
  memcpy(bytes + 5, p->params, p->len - 2);

  memset(bytes + 5 + (p->len - 2), calculate_checksum(p), sizeof(uint8_t));

  return bytes;
}

// takes a bite array of abitrary size and tries to
// parse it to a package
struct Package from_byte_array(uint8_t* arr) {
  size_t index = 0;
  assert(arr[index] == 0xAA);
  index++;
  assert(arr[index] == 0xAA);
  index++;

  uint8_t len = arr[index];
  index++;
  uint8_t id = arr[index];
  index++;
  uint8_t ctrl = arr[index];
  index++;

  struct Package p = {
    .len = len,
    .id = id,
    .ctrl = ctrl,
    .params = malloc(len - 2)
  };

  for (size_t i = 0; i < len - 2; i++) {
    p.params[i] = arr[index];
    index++;
  }

  return p;
}

// turn the serial connection into a raw serial connection
void with_makeraw(struct termios* tty) {
  cfmakeraw(tty);
  cfsetspeed(tty, B115200);

  tty->c_cflag &= ~PARENB; // disable parity and only one stop bit
  tty->c_cflag &= ~CSTOPB; // disable parity and only one stop bit

  tty->c_cc[VMIN] = 0; // block until either amount of data is available
  tty->c_cc[VTIME] = 10; // wait for 1 second on read
}

// returns a file descriptor of the serial port
int init_serial(char* port_name) {
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

  with_makeraw(&tty);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    close(serial_port);
    exit(EXIT_FAILURE);
  }
  return serial_port;
}


// writes the package to the given port
void write_package(int serial_port, struct Package* p) {
  uint8_t* package_bytes = to_byte_array(p);
  size_t array_len = get_array_len(p);
  printf("array_len = %ld\n", array_len);

  printf("Writing: ");
  for (int i = 0; i < array_len; i++) {
    printf("%02X", package_bytes[i]);
  }
  printf("\n");

  int res = write(serial_port, package_bytes, array_len);
  if (res != 0) {
    printf("write returned %d: %s\n", errno, strerror(errno));
  }
}

// prints a package (params loosy goosy as a string)
void print_package(struct Package* p) {
  printf("Parsed package:\n");
  printf(" p.id = %d\n", p->id);
  printf(" p.len = %d\n", p->len);
  printf(" p.ctrl = %d\n", p->ctrl);
  printf(" p.params = ");
  for (size_t i = 0; i < p->len - 2; i++) {
    printf("%c", p->params[i]);
  }
  printf("\n");
}

int main(void) {
  int serial_port = init_serial("/dev/ttyUSB0");
  struct Package p = {
    .id = 0,
    .len = 2,
    .ctrl = 0,
    .params = NULL
  };

  char* buf[256] = {0};

  write_package(serial_port, &p);

  int read_bytes = read(serial_port, &buf, sizeof(buf));
  printf("Read %d bytes\n", read_bytes);

  struct Package pp = from_byte_array((uint8_t*) &buf);
  print_package(&pp);

  close(serial_port);

  return EXIT_SUCCESS;
}
