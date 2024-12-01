#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "package.h"
// calculates the number of bytes that byte array needs
// in order for the package to to fit inside
//
// It is calculated as follows:
// - 2 bytes for the header 0xAA bytes
// - 1 byte for the params length
// - p->len bytes for id, ctrl and params bytes
// - 1 byte for the checksum
size_t get_array_len(struct Package *p) {
  return (sizeof(uint8_t) * 2) // Header bytes
         + sizeof(uint8_t)     // len
         + p->len              // payload length
         + sizeof(uint8_t);    // checksum
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

// `Allocates` an array to copy the package into.
//
// Must be freed at end of use!
uint8_t *to_byte_array(struct Package *p) {
  uint8_t *bytes = (uint8_t *)malloc(get_array_len(p));

  // set header bytes in array
  memset(bytes, 0xAA, sizeof(uint8_t) * 2);

  memcpy(bytes + 2, &p->len, sizeof(uint8_t));

  // copy payload into array
  memcpy(bytes + 3, &p->id, sizeof(uint8_t));
  memcpy(bytes + 4, &p->ctrl, sizeof(uint8_t));
  memcpy(bytes + 5, p->params, p->len - 2);

  memset(bytes + 5 + (p->len - 2), calculate_checksum(p), sizeof(uint8_t));

  return bytes;
}

// Takes a bite array of abitrary size and tries to
// parse it to a package
//
// ## Fields:
// * `arr: uint8_t*` - The received bytes to parse
//
// TODO: Validate checksum
struct Package from_byte_array(uint8_t *arr) {
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
      .len = len, .id = id, .ctrl = ctrl, .params = malloc(len - 2)};

  for (uint8_t i = 0; i < len - 2; i++) {
    p.params[i] = arr[index];
    index++;
  }

  return p;
}

// prints a package (params loosy goosy as a string)
void print_package(struct Package *p) {
  printf("Package {\n");
  printf("  p.id = %d\n", p->id);
  printf("  p.len = %d\n", p->len);
  printf("  p.ctrl = %d\n", p->ctrl);
  printf("  p.params = ");
  for (uint8_t i = 0; i < p->len - 2; i++) {
    printf("%c", p->params[i]);
  }
  printf("\n}\n");
}

void print_package_hex(struct Package *p) {
  printf("Package { \n");
  printf("  p.id = %d\n", p->id);
  printf("  p.len = %d\n", p->len);
  printf("  p.ctrl = %d\n", p->ctrl);
  printf("  p.params = ");
  for (uint8_t i = 0; i < p->len - 2; i++) {
    printf("%02X", p->params[i]);
  }
  printf("\n}\n");
}

// Frees the params memory.
void free_package(struct Package *p) { free(p->params); }
