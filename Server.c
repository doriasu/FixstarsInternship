#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>
#include <sys/neutrino.h>
#include<sys/iomsg.h>
#include <sys/netmgr.h>
#include <unistd.h>
struct adder {
  uint16_t types;
  uint8_t a;
  uint8_t b;
};
struct resulter {
  uint16_t types;
  uint8_t ans ;
};

int main(void) {
  int chid=ChannelCreate(0);
  printf("chid:%d\n",chid);
  printf("pid:%d\n",getpid());
  struct adder tmp;
  struct resulter ans;
  struct _msg_info info;
  uint8_t rcvid=MsgReceive(chid,&tmp,sizeof(tmp),&info);
  ans.ans=tmp.a+tmp.b;
  uint8_t status=0;
  ans.types=_IO_MAX + 2;
  if(tmp.a>=100||tmp.b>=100){
      status=-1;
    
  }
  if(MsgReply(rcvid,status,&ans,sizeof(ans))==-1){
      perror("クライアントが送信に失敗しました\n");
      return 0;

  }
  




  return 0;
}