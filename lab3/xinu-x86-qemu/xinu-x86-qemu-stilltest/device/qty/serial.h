#ifndef DEVICES_SERIAL_H
#define DEVICES_SERIAL_H

#include "qty.h"

void serial_init_queue (void);
void serial_putc (uint8_t);
void serial_notify (void);
void serial_interrupt (int);

#endif /* devices/serial.h */
