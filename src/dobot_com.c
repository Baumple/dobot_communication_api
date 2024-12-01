#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "dobot_types.h"
#include "package.h"
#include "serial.h"

#ifdef RUN_TESTS
#include "test.h"
#endif /* ifdef RUN_TESTS */

// Returns a `heap` allocated string with the device's serial number.
char *GetDeviceSN(int serial_port) {
  struct Package p = {.id = 0, .len = 2, .ctrl = 0, .params = NULL};
  write_package(serial_port, &p);

  return receive_string(serial_port);
}

// Returns a `heap` allocated string with the device name.
char *GetDeviceName(int serial_port) {
  struct Package p = {.id = 1, .len = 2, .ctrl = 0, .params = NULL};
  write_package(serial_port, &p);
  return receive_string(serial_port);
}

// retu
struct DobotVersion GetDeviceVersion(int serial_port) {
  struct Package p = {.id = 1, .len = 2, .ctrl = 0, .params = NULL};
  write_package(serial_port, &p);

  struct Package pp = receive_package(serial_port);

  struct DobotVersion v = {0};
  v.major = pp.params[0];
  v.minor = pp.params[1];
  v.revision = pp.params[2];

  free_package(&pp);

  return v;
}

struct DobotPose GetPose(int serial_port) {
  struct Package p = {.id = 10, .len = 2, .ctrl = 0, .params = NULL};

  write_package(serial_port, &p);
  struct Package pp = receive_package(serial_port);

  struct DobotPose pose = {0};

  memcpy(&pose, pp.params, pp.len - 2);

  free_package(&pp);

  return pose;
}

#ifdef RUN_TESTS
int main(void) { run_tests(); }
#else
int main(void) {
  int serial_port = init_serial("/dev/ttyUSB0");

  // clean up
  close(serial_port);

  return EXIT_SUCCESS;
}
#endif
