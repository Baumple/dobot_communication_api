#define RUN_TESTS

#ifdef RUN_TESTS
#include <stdint.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dobot_types.h"
#include "package.h"
#include "serial.h"

int init_mock(struct Package *p) {
  int fd = open("./mock_file", O_RDWR | O_CREAT, 0666);
  if (fd < 0) {
    printf("Could not open test file\n");
    exit(1);
  }

  write_package(fd, p);

  close(fd);

  fd = open("./mock_file", O_RDONLY);
  if (fd < 0) {
    printf("Could not reopen test file\n");
    exit(1);
  }

  return fd;
}

void test_GetDeviceSN() {
  struct Package initial = {.id = 0,
                            .len = 2 + strlen("hello"),
                            .ctrl = 0,
                            .params = (uint8_t *)"hello"};

  int serial_port = init_mock(&initial);
  if (serial_port < 0) {
    printf("Failed to create sn_mock file.");
    exit(1);
  }

  char *sn = receive_string(serial_port);
  assert(strcmp("hello", sn) == 0);

  // clean up
  close(serial_port);
  free(sn);
}

void test_GetDeviceVersion() {
  uint8_t version[] = {1, 2, 0};
  struct Package initial = {
      .len = 2 + (sizeof version / sizeof version[0]),
      .id = 2,
      .ctrl = 0,
      .params = version,
  };

  int fd = init_mock(&initial);
  struct Package p = receive_package(fd);

  uint8_t received_version[] = {
      p.params[0],
      p.params[1],
      p.params[2],
  };

  for (int i = 0; i < 3; i++) {
    assert(received_version[i] == version[i]);
  }

  close(fd);
  free_package(&p);
}

void test_GetPose() {
  struct DobotPose pose = {.x = 23.4,
                           .y = 12.2,
                           .z = 44.0,
                           .r = 2.0,
                           .jointAngle = {12.0, 98.2, 34.4, 13.37}};

  struct Package p = {.len = 2 + sizeof(struct DobotPose),
                      .id = 11,
                      .ctrl = 0,
                      .params = (uint8_t *)&pose};

  int fd = init_mock(&p);

  struct Package received = receive_package(fd);

  struct DobotPose *dp = (struct DobotPose *)received.params;

  assert(pose.x == dp->x);
  assert(pose.y == dp->y);
  assert(pose.z == dp->z);
  assert(pose.r == dp->r);

  for (int i = 0; i < 4; i++) {
    assert(pose.jointAngle[i] == dp->jointAngle[i]);
  }

  free_package(&received);
}

void run_tests() {
  printf("Running tests\n");
  test_GetDeviceSN();
  test_GetDeviceVersion();
  test_GetPose();
}
#endif /* ifdef RUN_TESTS */
