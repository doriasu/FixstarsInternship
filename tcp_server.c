#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char **argv) {
  int sd;
  int acc_sd;
  struct sockaddr_in addr;

  socklen_t sin_size = sizeof(struct sockaddr_in);
  struct sockaddr_in from_addr;

  char *buf = malloc(sizeof(char) * 10000000);
  int buf_size = 10000000;

  // 受信バッファの初期化
  memset(buf, 0, buf_size);

  // IPv4 TCP のソケットを作成
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  // 待ち受けるIPとポート番号を設定
  addr.sin_family = AF_INET;
  addr.sin_port = htons(50000);
  addr.sin_addr.s_addr = INADDR_ANY;

  // バインドする
  if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return -1;
  }

  // パケット受信待ち状態とする
  // 待ちうけキューを１０としている
  if (listen(sd, 10) < 0) {
    perror("listen");
    return -1;
  }

  // クライアントからコネクト要求が来るまで停止する
  // 以降、サーバ側は acc_sd を使ってパケットの送受信を行う
  if ((acc_sd = accept(sd, (struct sockaddr *)&from_addr, &sin_size)) < 0) {
    perror("accept");
    return -1;
  }
  for (int i = 0; i < 1; i++) {
    int yomikomi;
    char *shashin = malloc(sizeof(char) * 10000000);
    int youryou = 0;

    // パケット受信。パケットが到着するまでブロック
    while ((yomikomi = recv(acc_sd, buf, buf_size, 0)) > 0) {
      memcpy(shashin + youryou, buf, yomikomi);
      youryou += yomikomi;
    }
    int youryou_sub = youryou;
    /*char file_name[20];
    snprintf(file_name, 20, "sample_%d.jpg", i);
    int dest_fp = open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND,
                       S_IREAD | S_IWRITE);
    if (dest_fp == -1) {
      perror("書き込みファイルを開くのに失敗しました。\n");
      return 0;
    }*/
    char *buf_sub = shashin;
    int start = 0;
    int end = 0;
    for (int i = 0; i < youryou_sub - 2; i++) {
      if (buf_sub[i] == 'x' && buf_sub[i + 1] == 'x' && buf_sub[i + 2] == 'x') {
        end = i;
        break;
      }
    }
    int k = 0;
    for(int i=0;i<youryou_sub;i++){
      printf("%c",shashin[i]);
    }
    //うまく動けば勝ちなのはこいつ
    youryou_sub++;
    while (start != youryou_sub) {
      char file_name[20];
      snprintf(file_name, 20, "sample_%d.jpg", k);
      int dest_fp = open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND,
                         S_IREAD | S_IWRITE);
      if (dest_fp == -1) {
        perror("書き込みファイルを開くのに失敗しました。\n");
        return 0;
      }
      k++;
      int yomikomi = end - start;
      while (yomikomi > 0) {
        int kakikomi = write(dest_fp, buf_sub, yomikomi);
        //エラー処理
        if (kakikomi == -1) {
          if (errno == EINTR) {
          } else {
            perror("ファイルの書き込みに失敗しました。\n");
            return 0;
          }
        }
        buf_sub += kakikomi;
        yomikomi -= kakikomi;
      }
      start = end + 3;
      if (start != youryou_sub) {
        int x=end;
        for (int i=end; i < youryou_sub - 2; i++) {
          if (buf_sub[i] == 'x' && buf_sub[i + 1] == 'x' &&
              buf_sub[i + 2] == 'x') {
            end=i;
            break;
          }
        }
        if(x==end){
          end=youryou_sub-3;
        }
        buf_sub+=3;
      }else{
        break;
      }
      close(dest_fp);
      printf("start:%d end:%d youryou:%d\n",start,end,youryou_sub);

    }
    /*while (youryou > 0) {
      int kakikomi = write(dest_fp, buf_sub, youryou);
      printf("yomikomi:%d\n", yomikomi);
      printf("kakikomi:%d\n", kakikomi);

      //エラー処理
      if (kakikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの書き込みに失敗しました。\n");
          return 0;
        }
      }
      buf_sub += kakikomi;
      youryou -= kakikomi;
    }*/

    
  }

  // パケット送受信用ソケットのクローズ
  close(acc_sd);

  // 接続要求待ち受け用ソケットをクローズ
  close(sd);

  return 0;
}