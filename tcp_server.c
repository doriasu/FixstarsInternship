#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
int main(int argc, char **argv) {
  int sd;
  int acc_sd;
  struct sockaddr_in addr;

  socklen_t sin_size = sizeof(struct sockaddr_in);
  struct sockaddr_in from_addr;

  char *buf = malloc(sizeof(char) * 1000000);
  int buf_size = 1000000;

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
  for (int i = 0; i < 100; i++) {
    int yomikomi;
    // パケット受信。パケットが到着するまでブロック
    if ((yomikomi = recv(acc_sd, buf, buf_size, 0)) < 0) {
      perror("recv");
      return -1;
    }
    char file_name[20];
    snprintf(file_name, 20, "sample_%d.jpg", i);
    int dest_fp =
        open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (dest_fp == -1) {
      perror("書き込みファイルを開くのに失敗しました。\n");
      return 0;
    }
    while (yomikomi > 0) {
      int kakikomi = write(dest_fp, buf, yomikomi);

      //エラー処理
      if (kakikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの書き込みに失敗しました。\n");
          return 0;
        }
      }
      buf += kakikomi;
      yomikomi -= kakikomi;
    }
    close(dest_fp);
  }

  // パケット送受信用ソケットのクローズ
  close(acc_sd);

  // 接続要求待ち受け用ソケットをクローズ
  close(sd);

  return 0;
}