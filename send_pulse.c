#include <errno.h>
#include <stdio.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "header.h"
void send_pulse(int coid);
int main(void){
  char* path="/dev/mynull";
  int null_open=open(path,O_RDWR);
  if(null_open==-1){
    perror("mynullを開くのに失敗しました。\n");
    return 0;
  }
  send_pulse(null_open);
  if(ConnectDetach(null_open)==-1){
    perror("ファイルを閉じるのに失敗しました。\n");
  }
  return 0;


}
void send_pulse(int coid) {
  int pulse_err =
      MsgSendPulse(coid, sched_get_priority_max(SCHED_FIFO), PULSE_CODE, 0);
  if (pulse_err == -1) {
    perror("パルスの送信に失敗しました。\n");
  }
}