#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "header.h"

int main(void) {
  int chid = ChannelCreate(0);
  if (chid < 0) {
    perror("通信を開くのに失敗しました\n");
    return 0;
  }
  printf("chid:%d\n", chid);
  printf("pid:%ld\n", (long)getpid());

  int destroy_err;
  while (1) {
    struct add_pulse msg;
    struct resulter ans;
    struct _msg_info info;
    int rcvid = MsgReceive(chid, &msg, sizeof(msg), &info);
    if (rcvid == 0) {
      if (msg.pulse.code == PULSE_CODE) {
        printf("パルスを受信したのでアプリケーションを終了します。\n");
        break;
      } else {
        continue;
      }
    }

    if (rcvid == -1) {
      perror("受信に失敗しました。");
      continue;
    }
    if (msg.pulse.type != MESSAGE_CODE) {
      printf("正しい接続を確立できませんでした。\n");
      continue;
    }
    ans.ans = msg.add.a + msg.add.b;
    int status = 0;
    if (msg.add.a >= 100 || msg.add.b >= 100) {
      int msg_err = MsgError(rcvid, 1);
      if (msg_err == -1) {
        perror("エラーメッセージの送信に失敗しました。\n");
        continue;
      }
      printf("値は100より小さくしてください。\n");
      continue;

    } else if (MsgReply(rcvid, status, &ans, sizeof(ans)) == -1) {
      perror("クライアントが送信に失敗しました\n");
    }
  }
  destroy_err = ChannelDestroy(chid);
  if (destroy_err == -1) {
    perror("通信の切断に失敗しました。\n");
  }

  return 0;
}