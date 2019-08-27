#ifndef _HEADER_H_
#define _HEADER_H_
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define pulse_code _PULSE_CODE_MINAVAIL+1
#define message_code _IO_MAX + 1

struct adder {
  int a;
  int b;
};
struct resulter {
  int ans;
};
union add_pulse{
    struct _pulse pulse;
    struct adder add;
  };

#endif//_HEADER_H_
