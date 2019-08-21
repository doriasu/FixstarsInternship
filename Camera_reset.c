#include <hw/i2c.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define _XTAL_FREQ 4000000
void kakikomu(int *fd, uint8_t *address, uint8_t *atai) {
  int kakikomu_reg = spi_write(*fd, 0, address, 1);
  if (kakikomu_reg == -1) {
    perror("レジスタの指定に失敗しました。\n");
    return;
  }
  int kakikomu_val = spi_write(*fd, 0, atai, 1);
  if (kakikomu_val == -1) {
    perror("値の書き込みに失敗しました。\n");
    return;
  }
  return;
}

int yomikomu(int *fd, uint8_t *address) {
  int yomikomu_reg = spi_write(*fd, 0, address, 1);
  if (yomikomu_reg == -1) {
    perror("レジスタの指定に失敗しました。\n");
    return -1;
  }
  int tmp = 0x00;
  int yomikomu_val = spi_read(*fd, 0, &tmp, 1);
  if (yomikomu_val == -1) {
    perror("値の読み込みに失敗しました。\n");
    return -1;
  }
  return tmp;
}

int main(void) {
  //時間処理
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1e+8;

  int fd = spi_open("/dev/spi0");
  // setconfig
  uint32_t bits_per_word = SPI_MODE_CHAR_LEN_MASK & 8;
  spi_cfg_t cfg = {
      .mode = bits_per_word | SPI_MODE_BODER_MSB,
      .clock_rate = 4 * 1000 * 1000,  // 4MHz
  };
  int cfg_err = spi_setcfg(fd, 0, &cfg);
  if (cfg_err != EOK) {
    perror("接続に失敗しました\n");
    return 0;
  }
  // 1.レジスタ0x07に0x80を書き込む
  uint8_t add = 0x87;
  int add_er = spi_write(fd, 0, &add, 1);
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  uint8_t kakikomi = 0x80;
  int kakikomi_er = spi_write(fd, 0, &kakikomi, 1);
  if (kakikomi_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  // 2.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  // 3.レジスタ0x07に0x00を書き込む
  add = 0x87;
  add_er = spi_write(fd, 0, &add, 1);
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  kakikomi = 0x00;
  kakikomi_er = spi_write(fd, 0, &kakikomi, 1);
  if (kakikomi_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  // 4.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  // 5.レジスタ0x00に適当な値を書き込む
  add = 0x80;
  add_er = spi_write(fd, 0, &add, 1);
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  kakikomi = 0x55;
  kakikomi_er = spi_write(fd, 0, &kakikomi, 1);
  if (kakikomi_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  // 6.5.で書き込んだ値を読めるまで待ち、読む
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  add = 0x00;
  add_er = spi_write(fd, 0, &add, 1);
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  uint8_t yomikomi;
  int yomikomi_er = spi_read(fd, 0, &yomikomi, 1);
  if (yomikomi_er == -1) {
    perror("読み込みに失敗しました。\n");
    return 0;
  }
  printf("%d\n", yomikomi);

  return 0;
}