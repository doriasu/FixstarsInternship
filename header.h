#ifndef _HEADER_H_
#define _HEADER_H_

#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/types.h>
#include <unistd.h>
#include <devctl.h>
#define PULSE_CODE _PULSE_CODE_MINAVAIL+1
#define MYNULL_CODE 2
#define DCMD_MYNULL_KAKIKOMI __DIOT(_DCMD_MISC, MYNULL_CODE, char[256])



#endif//_HEADER_H_
