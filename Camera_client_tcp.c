#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "header.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("引数はipアドレスとポート番号と遅延時間(ms)です\n");
    return 0;
  }
  struct timespec timer;
  const char *camera = "/dev/Camera/satsuei";
  long interval_ms;
  char *endptr;
  interval_ms = strtoul(argv[3], &endptr, 10);
  if (*endptr != '\0') {
    perror("不正な文字が入力されています。数字msです。\n");
    return 0;
  }

  if (interval_ms > 999) {
    int ms = interval_ms % 1000;
    interval_ms /= 1000;
    timer.tv_sec = interval_ms;
    timer.tv_nsec = 1000 * 1000 * ms;
  } else {
    timer.tv_nsec = 1000 * 1000 * interval_ms;
  }

  const int fd = open(camera, O_RDONLY);
  if (fd < 0) {
    // エラー処理
    perror("cameraを開くのに失敗しました。\n");
  }

  //通信周り
  /* IP アドレス、ポート番号、ソケット */
  char *destination = argv[1];
  char *port_err;
  unsigned long port = strtol(argv[2], &port_err, 10);
  printf("HELLO\n");
  if (*port_err != '\0') {
    perror("ポート番号がおかしいです。\n");
    return 0;
  }
  int dstSocket;

  /* sockaddr_in 構造体 */
  struct sockaddr_in dstAddr;
  printf("HEHE\n");

  /* sockaddr_in 構造体のセット */
  memset(&dstAddr, 0, sizeof(dstAddr));
  dstAddr.sin_port = htons(port);
  dstAddr.sin_family = AF_INET;
  dstAddr.sin_addr.s_addr = inet_addr(destination);

  /* ソケット生成 */
  dstSocket = socket(AF_INET, SOCK_STREAM, 0);

  /* 接続 */
  printf("Trying to connect to %s: \n", destination);
  connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr));

  for (int i = 0; i < 1000; ++i) {
    int yomikomi = 0;
    char buf[10000];

    //読み込み0になる問題
    // while(c>0){if(c==-1)}はc==-1が常に偽になってしまうため修正した(注意)
    while ((yomikomi = read(fd, buf, sizeof(buf))) != 0) {
      if (yomikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの読み込みに失敗しました。\n");
          return 0;
        }
      }
      printf("%d\n", yomikomi);

      char *buf_sub = buf;

      while (yomikomi > 0) {
        int kakikomi = send(dstSocket, buf, yomikomi, 0);

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
        printf("%d\n", yomikomi);
      }
    }
    //画像の終端にダミーコードを送りつけておき、識別する。
    char *dummy = "xxx";
    send(dstSocket, dummy, 3, 0);

    clock_nanosleep(CLOCK_MONOTONIC, 0, &timer, NULL);
    // 次の撮影があるなら interval_ms だけ待機する
  } 
  

  close(fd);
  return 0;
}
