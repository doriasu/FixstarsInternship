#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char **argv) {
  char *src = argv[1];
  char *dest = argv[2];
  if (argc != 3) {
    printf("引数はソースファイル名と出力ファイル名の2つにしてください。\n");
    return 0;
  }
  int src_fp = open(src, O_RDONLY);
  if (src_fp == -1) {
    perror("ソースファイル名に不備があります。\n");
    return 0;
  }
  int dest_fp = open(dest, O_WRONLY | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
  if (dest_fp == -1) {
    perror("すでに存在しているファイル名です。変更してください。\n");
    return 0;
  }
  int yomikomi;
  char buf[1024];
  // while(c>0){if(c==-1)}はc==-1が常に偽になってしまうため修正した(注意)
  while ((yomikomi = read(src_fp, buf, sizeof(buf))) != 0) {
    if (yomikomi == -1) {
      if (errno == EINTR) {
      } else {
        perror("ファイルの読み込みに失敗しました。\n");
        unlink(dest);
        return 0;
      }
    }
    /*write(dest_fp,buf,sizeof(buf))にすると読み込んだサイズに関係なく
    (そのindexのbufに文字がなかったとしてもあるものとして)書き込んでしまうため
    余計なよくわからないものが書き込まれる可能性があるためyomikomiの
    文字数までに書き込み文字数を制限する必要がある。*/
    const char *buf_sub = buf;
    while (yomikomi > 0) {
      int kakikomi = write(dest_fp, buf_sub, yomikomi);
      //エラー処理
      if (kakikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの書き込みに失敗しました。\n");
          unlink(dest);
          return 0;
        }
      }
      buf_sub += kakikomi;
      yomikomi -= kakikomi;
    }
  }
  close(src_fp);
  close(dest_fp);
}
