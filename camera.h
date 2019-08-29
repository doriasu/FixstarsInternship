#include "header.h"
#include <fcntl.h>
#include <hw/i2c.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int Camera_setup(int fd);
static int i2c_write(int fd, uint32_t addr, const void *yomikomi_reg,
                     size_t len);
char *i2c_read(int fd, uint32_t addr, const void *yomikomi_reg, size_t len);
uint32_t satsuei();
char file_path[100];
int satsuei_flag;
uint32_t gazou_size;
int fd_spi;
//カメラ周り
uint8_t (*kaizoudo)[2];
int kaizoudo_size;
int kaizoudo_now;
//以下カメラに関する関数
int Camera_setup(int fd) {
  //解像度640*480
  kaizoudo_size = 0;
  kaizoudo = gazou_640;
  kaizoudo_size = sizeof(gazou_640);
  kaizoudo_now=4;
  // SPIパート
  uint32_t bits_per_word = SPI_MODE_CHAR_LEN_MASK & 8;
  spi_cfg_t cfg = {
      .mode = bits_per_word | SPI_MODE_BODER_MSB | SPI_MODE_CKPOL_HIGH,
      .clock_rate = 4 * 1000 * 1000,  // 4MHz
  };
  int cfg_err = spi_setcfg(fd_spi, 0, &cfg);
  if (cfg_err != EOK) {
    perror("接続に失敗しました\n");
    return -1;
  }
  // 1.レジスタ0x07に0x80を書き込む
  uint8_t kakikomi[2] = {0x87, 0x80};
  int add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return -1;
  }

  // 2.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0,
                  &(struct timespec){.tv_nsec = 100 * 1000 * 1000},
                  NULL);  // 100ms
  // 3.レジスタ0x07に0x00を書き込む
  kakikomi[0] = 0x87;
  kakikomi[1] = 0x00;
  add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return -1;
  }

  // 4.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0,
                  &(struct timespec){.tv_nsec = 100 * 1000 * 1000},
                  NULL);  // 100ms

  // I2Cパート

  // 1.レジスタ0xffに0x01を書き込む
  kakikomi[0] = 0xff;
  kakikomi[1] = 0x01;

  if (i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi)) == -1) {
    perror("書き込みに失敗しました。\n");
    return -1;
  }

  //プロダクトIDの確認
  uint8_t product[1] = {0x0a};
  char *data_a = i2c_read(fd, 0x30, product, sizeof(product));
  if (data_a[0] != 38) {
    printf("プロダクトID(0x0a)の値が違います\n");
    return -1;
  }
  printf("%d\n", data_a[0]);
  product[0] = 0x0b;
  char *data_b = i2c_read(fd, 0x30, product, sizeof(product));
  if (data_b[0] != 66) {
    printf("プロダクトID(0x0b)の値が違います\n");
    return -1;
  }
  printf("%d\n", data_b[0]);

  // 2.0x12に0x80をかきこむ
  kakikomi[0] = 0x12;
  kakikomi[1] = 0x80;

  //実際に書き込んでいく
  if (i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi)) == -1) {
    perror("書き込みに失敗しました。\n");
    return -1;
  }

  clock_nanosleep(CLOCK_MONOTONIC, 0,
                  &(struct timespec){.tv_nsec = 10 * 1000 * 1000},
                  NULL);  // 10ms
  // 3.設定値の書き込み
  for (int i = 0; i < (int)sizeof(settei) / 2; i++) {
    kakikomi[0] = settei[i][0];
    kakikomi[1] = settei[i][1];
    /*printf("%d\n",kakikomi[0]);
    printf("%d\n",kakikomi[1]);*/
    add_er = i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi));
    if (add_er == -1) {
      perror("書き込みに失敗しました。\n");
      return -1;
    }
  }
  //画像のピクセルについての設定値の書き込み
  for (int i = 0; i < kaizoudo_size / 2; i++) {
    kakikomi[0] = kaizoudo[i][0];
    kakikomi[1] = kaizoudo[i][1];
    add_er = i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi));
    if (add_er == -1) {
      perror("書き込みに失敗しました。\n");
      return -1;
    }
  }
  return 0;
}

static int i2c_write(int fd, uint32_t addr, const void *yomikomi_reg,
                     size_t len) {
  i2c_send_t *buf = malloc(sizeof(i2c_send_t) + len);
  if (!buf) {
    perror("書き込みに失敗しました.\n");
    return -1;
  }
  buf->slave.addr = addr;
  buf->slave.fmt = I2C_ADDRFMT_7BIT;
  buf->len = len;
  buf->stop = 1;  // 0 にするとトランザクション継続
  memcpy((char *)buf + sizeof(i2c_send_t), yomikomi_reg, len);
  int err = devctl(fd, DCMD_I2C_SEND, buf, sizeof(i2c_send_t) + len, NULL);
  free(buf);
  if (err != EOK) {
    perror("書き込みに失敗しました\n");
    return -1;
  }
  return err;
}

char *i2c_read(int fd, uint32_t addr, const void *yomikomi_reg, size_t len) {
  char *err = "ERROR";
  int write_err = i2c_write(fd, addr, yomikomi_reg, len);
  if (write_err == -1) {
    return err;
  }
  uint8_t yomikomi[1];
  i2c_recv_t *buf_rec = malloc(sizeof(i2c_recv_t) + sizeof(yomikomi));
  if (!buf_rec) {
    perror("読み込みに失敗しましたf\n");
    return err;
  }
  buf_rec->slave.addr = addr;
  buf_rec->slave.fmt = I2C_ADDRFMT_7BIT;
  buf_rec->len = sizeof(yomikomi);
  buf_rec->stop = 1;  // 0 にするとトランザクション継続
  int err_dev = devctl(fd, DCMD_I2C_RECV, buf_rec,
                       sizeof(i2c_recv_t) + sizeof(yomikomi), NULL);
  if (err_dev != EOK) {
    perror("読み込みに失敗しましたg\n");
    return err;
  }
  char *data = (char *)buf_rec + sizeof(i2c_recv_t);

  free(buf_rec);
  return data;
}

uint32_t satsuei() {
  uint8_t kakikomi[2];
  kakikomi[0] = 0x84;
  kakikomi[1] = 0x01;
  int add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  clock_nanosleep(CLOCK_MONOTONIC, 0,
                  &(struct timespec){.tv_nsec = 10 * 1000 * 1000},
                  NULL);  // 10ms
  // 0x04に0x02を書き込む
  kakikomi[0] = 0x84;
  kakikomi[1] = 0x02;
  add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }
  //撮影完了か調べる
  // 0x11はdummy
  uint8_t yomikomi[2];
  yomikomi[0] = 0;
  yomikomi[1] = 0;
  while ((yomikomi[1] & 8) == 0) {
    uint8_t reg[2] = {0x41, 0x11};
    if (spi_xchange(fd_spi, 0, reg, yomikomi, sizeof(yomikomi)) < 0) {
      perror("読み込みに失敗しましたa\n");
      return 0;
    }
  }
  //データバイトの取得
  uint8_t reg_42[2];
  uint8_t reg_43[2];
  uint8_t reg_44[2];
  uint8_t reg[2] = {0x42, 0x11};
  if (spi_xchange(fd_spi, 0, reg, reg_42, sizeof(reg_42)) < 0) {
    perror("読み込みに失敗しましたb\n");
    return 0;
  }
  reg[0] = 0x43;
  if (spi_xchange(fd_spi, 0, reg, reg_43, sizeof(reg_43)) < 0) {
    perror("読み込みに失敗しましたc\n");
    return 0;
  }
  reg[0] = 0x44;
  if (spi_xchange(fd_spi, 0, reg, reg_44, sizeof(reg_44)) < 0) {
    perror("読み込みに失敗しましたd\n");
    return 0;
  }
  //最上位ビットの読み捨て(そういう仕様)
  const uint32_t size = (((uint32_t)(reg_44[1]) << 16) |
                         ((uint32_t)(reg_43[1]) << 8) | reg_42[1]) &
                        0x7FFFFF;

  return size;
}
