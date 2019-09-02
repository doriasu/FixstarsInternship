#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <unistd.h>
#include "header.h"
static void usage(void) {
  printf(
      "Usage: shot [-r resolution] [-d device] [-i interval] file1 [file2 "
      "...]\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  const char *camera = "/dev/Camera";
  char resolution[20] = "A";
  long interval_ms = 1000;
  char *endptr;
  int ch;
  while ((ch = getopt(argc, argv, "d:i:r:")) != -1) {
    switch (ch) {
      case 'd':
        camera = optarg;
        break;
      case 'i':
        // optarg をパースしてミリ秒を取得、interval_ms に代入
        interval_ms = strtoul(optarg, &endptr, 10);
        if (*endptr != '\0') {
          perror("不正な文字が入力されています。数字msです。\n");
          return 0;
        }

        break;
      case 'r':
        // optarg をパースして解像度を取得、resolution に代入
        strncpy(resolution, optarg, sizeof(resolution));
        break;
      default:
        usage();
    }
  }

  const int fd = open(camera, O_RDONLY);
  if (fd < 0) {
    // エラー処理
    perror("cameraを開くのに失敗しました。\n");
  }

  // 解像度設定
  if (strcmp(resolution, "CAMERA_RES_160X120") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_176X144") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_160X120") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_352X288") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_640X480") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_800X600") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_1024X768") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_1280X1024") == 0) {
  } else if (strcmp(resolution, "CAMERA_RES_1600X1200") == 0) {
  } else if (strcmp(resolution, "A") == 0) {
  } else {
    printf("その解像度は存在しません。\n");
    return 0;
  }
  if (strcmp(resolution, "A") != 0) {
    if (devctl(fd, DCMD_CAMERA_SETRES, STR(resolution), sizeof(STR(resolution)),
               NULL) != EOK) {
      perror("devctlにエラーが発生しました。\n");
    } else {
      printf("送ったよ\n");
    }
  }

  for (int i = optind; i < argc; ++i) {
    // 撮影を行い argv[i] のファイルに保存

    int dest_fp =
        open(argv[i], O_WRONLY | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
    if (dest_fp == -1) {
      perror("すでに存在しているファイル名です。変更してください。\n");
      return 0;
    }
    int yomikomi = 0;
    char buf[100000];
    //読み込み0になる問題
    // while(c>0){if(c==-1)}はc==-1が常に偽になってしまうため修正した(注意)
    while ((yomikomi = read(fd, buf, sizeof(buf))) != 0) {
      if (yomikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの読み込みに失敗しました。\n");
          unlink(argv[i]);
          return 0;
        }
      }
      char *buf_sub = buf;
      while (yomikomi > 0) {
        int kakikomi = write(dest_fp, buf_sub, yomikomi);

        //エラー処理
        if (kakikomi == -1) {
          if (errno == EINTR) {
          } else {
            perror("ファイルの書き込みに失敗しました。\n");
            unlink(argv[i]);
            return 0;
          }
        }
        buf_sub += kakikomi;
        yomikomi -= kakikomi;
      }
    }
    close(dest_fp);
    clock_nanosleep(CLOCK_MONOTONIC, 0,
                    &(struct timespec){.tv_nsec = interval_ms * 1000 * 1000},
                    NULL);
    // 次の撮影があるなら interval_ms だけ待機する
  }

  close(fd);
  return 0;
}
