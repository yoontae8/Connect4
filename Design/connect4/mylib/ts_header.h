#ifndef _TSRECV_
#define _TSRECV_

#include <linux/input.h>
#include <fcntl.h>
#define EVDEVFILE "/dev/input/event1"

int ts_read(int *xcrd, int *ycrd);
int get_colnum(int xcrd);

#endif
