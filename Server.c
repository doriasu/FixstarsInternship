#include "header.h"
struct adder {
  uint16_t types;
  int a;
  int b;
};
struct resulter {
  int ans;
};

int main(void) {
  int chid = ChannelCreate(0);
  if (chid < 0) {
    perror("通信を開くのに失敗しました\n");
    return 0;
  }
  printf("chid:%d\n", chid);
  printf("pid:%ld\n", (long)getpid());
  struct adder tmp;
  struct resulter ans;
  struct _msg_info info;
  while (1) {
    int rcvid = MsgReceive(chid, &tmp, sizeof(tmp), &info);
    if (rcvid == -1) {
      perror("受信に失敗しました。");
      ChannelDestroy(chid);
      return 0;
    }
    if (tmp.types != _IO_MAX + 1) {
      printf("正しい接続を確立できませんでした。\n");
      ChannelDestroy(chid);
      return 0;
    }
    ans.ans = tmp.a + tmp.b;
    int status = 0;
    if (tmp.a >= 100 || tmp.b >= 100) {
      MsgError(rcvid, 1);
      printf("値は100より小さくしてください。\n");
      ChannelDestroy(chid);

      return 0;

    } else if (MsgReply(rcvid, status, &ans, sizeof(ans)) == -1) {
      perror("クライアントが送信に失敗しました\n");
      ChannelDestroy(chid);
      return 0;
    }
  }
  ChannelDestroy(chid);

  return 0;
}