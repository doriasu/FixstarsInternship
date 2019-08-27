#ifndef _HEADER_H_
#define _HEADER_H_
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define PULSE_CODE _PULSE_CODE_MINAVAIL+1
#define MESSAGE_CODE _IO_MAX + 1

struct adder {
  int a;
  int b;
};
struct resulter {
  int ans;
};
struct add_pulse{
    struct _pulse pulse;
    struct adder add;
  };

#endif//_HEADER_H_
