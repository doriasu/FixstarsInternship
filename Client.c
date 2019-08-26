#include "header.h"
struct adder {
  uint16_t types;
  int a;
  int b;
};
struct resulter {
  int ans;
};

int main(int argc, char** argv) {
  if(argc!=5){
    printf("引数は<足す数その1><足す数その2><pid><chid>です。正しく入力してください。");
    return 0;
  }
  struct adder serv;
  uint16_t message_code = _IO_MAX + 1;
  char* endptr;
  serv.types = message_code;
  serv.a = strtoul(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    perror("100までの数字を2つ入力してください。\n");
    return 0;
  }
  serv.b = strtoul(argv[2], &endptr, 10);
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
  if(coid==-1){
    perror("サーバーとの接続に失敗しました\n");
    return 0;
  }
  struct resulter ans;
  
  int send_err = MsgSend(coid, &serv, sizeof(serv), &ans, sizeof(ans));
  if (send_err < 0 ) {
    perror("正しい値(100以下)を入力してください\n");
    ConnectDetach(coid);
    return 0;
  }
  printf("合計値:%d\n", ans.ans);
  ConnectDetach(coid);
  return 0;

}