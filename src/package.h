#pragma once
#include <stdint.h>
#include <strings.h>

// The package struct used to communicate with the dobot
//
// WARN: Len is always at least two longer than the params array which is
// for some reason required by the communication protocol.
//
// ## Fields:
//   * `len: uint8_t` - Represents the length of the payload in bytes. (2 +
//   len(params))
//   * `id: uint8_t` - The id of the command.
//   * `ctrl: uint8_t` - Control parameters: first bit is `rw`, `second is
//   isQueued`
//   * `params: uint8_t*` - An array of with the parameters of the command.
struct Package {
  uint8_t len;
  // Always 2 + (size of params)

  uint8_t id; // The id of the command.

  uint8_t ctrl; // Control bits:
                //  - The first byte is read_write
                //  - The second byte is isQueued

  uint8_t *params; // The command's parameters to sent
};

size_t get_array_len(struct Package *p);
uint8_t calculate_checksum(struct Package *p);
uint8_t *to_byte_array(struct Package *p);
struct Package from_byte_array(uint8_t *arr);
void print_package(struct Package *p);
void print_package_hex(struct Package *p);
void free_package(struct Package *p);
