#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include "header.h"

int main(int argc, char** argv) {
  if (argc != 5) {
    printf(
        "引数は<足す数その1><足す数その2><pid><chid>"
        "です。正しく入力してください。\n");
  }
  union add_pulse msg;
  char* endptr;
  msg.pulse.type = message_code;
  msg.add.a = strtoul(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    perror("100までの数字を2つ入力してください。\n");
    return 0;
  }
  msg.add.b = strtoul(argv[2], &endptr, 10);
  if (*endptr != '\0') {
    perror("100までの数字を2つ入力してください。\n");
    return 0;
  }
  pid_t pid = strtoul(argv[3], &endptr, 10);
  if (*endptr != '\0') {
    perror("pidを入力してください。\n");
    return 0;
  }
  int chid = strtoul(argv[4], &endptr, 10);
  if (*endptr != '\0') {
    perror("chidを入力してください。\n");
    return 0;
  }
  //接続
  int coid = ConnectAttach(ND_LOCAL_NODE, pid, chid, _NTO_SIDE_CHANNEL, 0);
  if (coid == -1) {
    perror("サーバーとの接続に失敗しました\n");
    return 0;
  }
  struct resulter ans;
  
  if (argc != 5) {
    printf("引数は2つの100より小さい整数とpidとchidです\n");
    struct _pulse err;
    err.code = pulse_code;
    err.type=message_code;
    // 0でいいのか???
    int pulse_err =
        MsgSendPulse(coid, sched_get_priority_max(SCHED_FIFO), err.code, 0);
    if (pulse_err == -1) {
      perror("パルスの送信に失敗しました。\n");
      return 0;
    }
  }
  int send_err = MsgSend(coid, &msg, sizeof(msg), &ans, sizeof(ans));
  int detach_err;
  if (send_err < 0) {
    perror("正しい値(100以下)を入力してください\n");
    detach_err = ConnectDetach(coid);
    if (detach_err == -1) {
      perror("通信の切断に失敗しました。\n");
    }
    return 0;
  }
  printf("合計値:%d\n", ans.ans);
  detach_err = ConnectDetach(coid);
  if (detach_err == -1) {
    perror("通信の切断に失敗しました。\n");
  }
  return 0;
}