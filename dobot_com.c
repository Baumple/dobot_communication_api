#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

void with_makeraw(struct termios* tty) {
  cfmakeraw(tty);
}

void with_manual(struct termios* tty) {
  tty->c_cflag &= ~PARENB; // disable parity
  tty->c_cflag &= ~(PARENB | CSTOPB); // disable parity and only one stop bit
  tty->c_cflag &= ~CSIZE;
  tty->c_cflag |= CS8; // eight bits per byte
  tty->c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
  tty->c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines
  tty->c_lflag &= ~ICANON; // disable canonical mode
  tty->c_lflag &= ~(ECHO | ECHOE | ECHONL | ISIG);
  tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

  // Disable any special handling of received bytes
  tty->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
      |INLCR|IGNCR|ICRNL);
  // Prevent special interpretation of output bytes (e.g. newline chars)
  // Prevent conversion of newline to carriage return/line feed
  tty->c_oflag &= ~(OPOST | ONLCR); 

  tty->c_cc[VTIME] = 10; // wait for 1 second on read
  tty->c_cc[VMIN] = 0; // block until either amount of data is available

  // baud rate
  cfsetspeed(tty, B115200);
  // cfsetospeed(&tty, B115200);
}

// returns a file descriptor of the serial port
int init_serial(char* port_name) {
  struct termios tty; // communication settings

  int serial_port = open(port_name, O_RDWR);

  if (serial_port < 0) {
    printf("Error %d while opening serial port: %s\n", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  if (tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  with_makeraw(&tty); // or with `with_manual`

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }
  return serial_port;
}

struct Package {
  uint8_t len;

  uint8_t id;
  uint8_t ctrl; // fist bit is rw, second bit is is_queued

  uint8_t* params;
};

size_t get_array_len(struct Package* p) {
  return (sizeof(uint8_t) * 2) // Header bytes
         + p->len // payload length
         + (sizeof(uint8_t)); // checksum
}

uint8_t calculate_checksum(struct Package *p) {
  uint8_t result = p->id + p->ctrl;

  // add up params
  uint8_t param_len = p->len - 2;
  for (size_t i = 0; i < param_len; i++) {
    result += p->params[i];
  }

  return (256 - result);
}

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

void test_bin() {
  char* params = "test";
  struct Package p = {
    .len = 2 + strlen(params),
    .id = 0,
    .ctrl = 0,
    .params = (uint8_t*) params,
  };

  printf("Original package:\n");
  printf(" p.id = %d\n", p.id);
  printf(" p.len = %d\n", p.len);
  printf(" p.ctrl = %d\n", p.ctrl);
  printf(" p.params = ");
  for (size_t i = 0; i < p.len - 2; i++) {
    printf("%c", p.params[i]);
  }
  printf("\n");

  uint8_t* arr = to_byte_array(&p);

  // print parsed 
  struct Package pp = from_byte_array(arr);
  printf("Parsed package:\n");
  printf(" pp.id = %d\n", pp.id);
  printf(" pp.len = %d\n", pp.len);
  printf(" pp.ctrl = %d\n", pp.ctrl);
  printf(" pp.params = ");
  for (size_t i = 0; i < pp.len - 2; i++) {
    printf("%c", pp.params[i]);
  }
  printf("\n");
}

int main(void) {
  test_bin();
  int serial_port = init_serial("/dev/ttyUSB0");

  char buf[256] = {0};

  int read_bytes = read(serial_port, &buf, sizeof(buf)); // and write(serial_port)

  close(serial_port);

  return EXIT_SUCCESS;
}
