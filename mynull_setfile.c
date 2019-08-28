#include <stdio.h>
#include <unistd.h>
#include "header.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("引数にはファイルパスのみ指定してください。\n");
    return 0;
  }
  char *path = "/dev/mynull";
  int null_open = open(path, O_RDWR);
  if (null_open == -1) {
    perror("mynullを開くのに失敗しました。\n");
    return 0;
  }
  char file_path[256];
  sprintf(file_path,argv[1]);
  if (devctl(null_open,MYNULL_CODE,file_path,sizeof(file_path),NULL) != EOK) {
    perror("devctlにエラーが発生しました。\n");
  }

  return 0;
}