#pragma once
#include "package.h"
#include <termios.h>

int init_serial(char *port_name);
void make_raw(struct termios *tty);
void write_package(int serial_port, struct Package *p);
struct Package receive_package(int serial_port);
char* receive_string(int serial_port);
