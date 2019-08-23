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
static int i2c_write(int fd, uint32_t addr, const void *data, size_t len);
static int i2c_read(int fd, uint32_t addr, const void *yomikomi_reg,size_t len);
int main(void) {
  //設定値
  uint8_t settei[206][2] = {{0xff, 0x00},
                            {0x2c, 0xff},
                            {0x2e, 0xdf},
                            {0xff, 0x01},
                            {0x3c, 0x32},
                            {0x11, 0x00},
                            {0x09, 0x02},
                            {0x04, 0x28},
                            {0x13, 0xe5},
                            {0x14, 0x48},
                            {0x2c, 0x0c},
                            {0x33, 0x78},
                            {0x3a, 0x33},
                            {0x3b, 0xfB},
                            {0x3e, 0x00},
                            {0x43, 0x11},
                            {0x16, 0x10},
                            {0x39, 0x92},
                            {0x35, 0xda},
                            {0x22, 0x1a},
                            {0x37, 0xc3},
                            {0x23, 0x00},
                            {0x34, 0xc0},
                            {0x36, 0x1a},
                            {0x06, 0x88},
                            {0x07, 0xc0},
                            {0x0d, 0x87},
                            {0x0e, 0x41},
                            {0x4c, 0x00},
                            {0x48, 0x00},
                            {0x5B, 0x00},
                            {0x42, 0x03},
                            {0x4a, 0x81},
                            {0x21, 0x99},
                            {0x24, 0x40},
                            {0x25, 0x38},
                            {0x26, 0x82},
                            {0x5c, 0x00},
                            {0x63, 0x00},
                            {0x61, 0x70},
                            {0x62, 0x80},
                            {0x7c, 0x05},
                            {0x20, 0x80},
                            {0x28, 0x30},
                            {0x6c, 0x00},
                            {0x6d, 0x80},
                            {0x6e, 0x00},
                            {0x70, 0x02},
                            {0x71, 0x94},
                            {0x73, 0xc1},
                            {0x12, 0x40},
                            {0x17, 0x11},
                            {0x18, 0x43},
                            {0x19, 0x00},
                            {0x1a, 0x4b},
                            {0x32, 0x09},
                            {0x37, 0xc0},
                            {0x4f, 0x60},
                            {0x50, 0xa8},
                            {0x6d, 0x00},
                            {0x3d, 0x38},
                            {0x46, 0x3f},
                            {0x4f, 0x60},
                            {0x0c, 0x3c},
                            {0xff, 0x00},
                            {0xe5, 0x7f},
                            {0xf9, 0xc0},
                            {0x41, 0x24},
                            {0xe0, 0x14},
                            {0x76, 0xff},
                            {0x33, 0xa0},
                            {0x42, 0x20},
                            {0x43, 0x18},
                            {0x4c, 0x00},
                            {0x87, 0xd5},
                            {0x88, 0x3f},
                            {0xd7, 0x03},
                            {0xd9, 0x10},
                            {0xd3, 0x82},
                            {0xc8, 0x08},
                            {0xc9, 0x80},
                            {0x7c, 0x00},
                            {0x7d, 0x00},
                            {0x7c, 0x03},
                            {0x7d, 0x48},
                            {0x7d, 0x48},
                            {0x7c, 0x08},
                            {0x7d, 0x20},
                            {0x7d, 0x10},
                            {0x7d, 0x0e},
                            {0x90, 0x00},
                            {0x91, 0x0e},
                            {0x91, 0x1a},
                            {0x91, 0x31},
                            {0x91, 0x5a},
                            {0x91, 0x69},
                            {0x91, 0x75},
                            {0x91, 0x7e},
                            {0x91, 0x88},
                            {0x91, 0x8f},
                            {0x91, 0x96},
                            {0x91, 0xa3},
                            {0x91, 0xaf},
                            {0x91, 0xc4},
                            {0x91, 0xd7},
                            {0x91, 0xe8},
                            {0x91, 0x20},
                            {0x92, 0x00},
                            {0x93, 0x06},
                            {0x93, 0xe3},
                            {0x93, 0x05},
                            {0x93, 0x05},
                            {0x93, 0x00},
                            {0x93, 0x04},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x93, 0x00},
                            {0x96, 0x00},
                            {0x97, 0x08},
                            {0x97, 0x19},
                            {0x97, 0x02},
                            {0x97, 0x0c},
                            {0x97, 0x24},
                            {0x97, 0x30},
                            {0x97, 0x28},
                            {0x97, 0x26},
                            {0x97, 0x02},
                            {0x97, 0x98},
                            {0x97, 0x80},
                            {0x97, 0x00},
                            {0x97, 0x00},
                            {0xc3, 0xed},
                            {0xa4, 0x00},
                            {0xa8, 0x00},
                            {0xc5, 0x11},
                            {0xc6, 0x51},
                            {0xbf, 0x80},
                            {0xc7, 0x10},
                            {0xb6, 0x66},
                            {0xb8, 0xA5},
                            {0xb7, 0x64},
                            {0xb9, 0x7C},
                            {0xb3, 0xaf},
                            {0xb4, 0x97},
                            {0xb5, 0xFF},
                            {0xb0, 0xC5},
                            {0xb1, 0x94},
                            {0xb2, 0x0f},
                            {0xc4, 0x5c},
                            {0xc0, 0x64},
                            {0xc1, 0x4B},
                            {0x8c, 0x00},
                            {0x86, 0x3D},
                            {0x50, 0x00},
                            {0x51, 0xC8},
                            {0x52, 0x96},
                            {0x53, 0x00},
                            {0x54, 0x00},
                            {0x55, 0x00},
                            {0x5a, 0xC8},
                            {0x5b, 0x96},
                            {0x5c, 0x00},
                            {0xd3, 0x00},
                            {0xc3, 0xed},
                            {0x7f, 0x00},
                            {0xda, 0x00},
                            {0xe5, 0x1f},
                            /*{0xe1, 0x67},*/ {0xe0, 0x00},
                            {0xdd, 0x7f},
                            {0x05, 0x00},
                            {0x12, 0x40},
                            {0xd3, 0x04},
                            {0xc0, 0x16},
                            {0xC1, 0x12},
                            {0x8c, 0x00},
                            {0x86, 0x3d},
                            {0x50, 0x00},
                            {0x51, 0x2C},
                            {0x52, 0x24},
                            {0x53, 0x00},
                            {0x54, 0x00},
                            {0x55, 0x00},
                            {0x5A, 0x2c},
                            {0x5b, 0x24},
                            {0x5c, 0x00},
                            {0xFF, 0x00},
                            {0x05, 0x00},
                            {0xDA, 0x10},
                            {0xD7, 0x03},
                            {0xDF, 0x00},
                            {0x33, 0x80},
                            {0x3C, 0x40},
                            /*{0xe1, 0x77},*/ {0x00, 0x00},
                            {0xe0, 0x14},
                            /*{0xe1, 0x77},*/ {0xe5, 0x1f},
                            {0xd7, 0x03},
                            {0xda, 0x10},
                            {0xe0, 0x00},
                            {0xFF, 0x01},
                            {0x04, 0x08},
                            {0xff, 0x01},
                            {0x15, 0x00}};
  //解像度640*480
  uint8_t gazou[40][2] = {
      {0xff, 0x01}, {0x11, 0x01}, {0x12, 0x00}, {0x17, 0x11}, {0x18, 0x75},
      {0x32, 0x36}, {0x19, 0x01}, {0x1a, 0x97}, {0x03, 0x0f}, {0x37, 0x40},
      {0x4f, 0xbb}, {0x50, 0x9c}, {0x5a, 0x57}, {0x6d, 0x80}, {0x3d, 0x34},
      {0x39, 0x02}, {0x35, 0x88}, {0x22, 0x0a}, {0x37, 0x40}, {0x34, 0xa0},
      {0x06, 0x02}, {0x0d, 0xb7}, {0x0e, 0x01}, {0xff, 0x00}, {0xe0, 0x04},
      {0xc0, 0xc8}, {0xc1, 0x96}, {0x86, 0x3d}, {0x50, 0x89}, {0x51, 0x90},
      {0x52, 0x2c}, {0x53, 0x00}, {0x54, 0x00}, {0x55, 0x88}, {0x57, 0x00},
      {0x5a, 0xa0}, {0x5b, 0x78}, {0x5c, 0x00}, {0xd3, 0x04}, {0xe0, 0x00}};

  //時間処理
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 10000000;
  // SPIパート
  // setconfig
  int fd_spi = spi_open("/dev/spi0");
  uint32_t bits_per_word = SPI_MODE_CHAR_LEN_MASK & 8;
  spi_cfg_t cfg = {
      .mode = bits_per_word | SPI_MODE_BODER_MSB,
      .clock_rate = 4 * 1000 * 1000,  // 4MHz
  };
  int cfg_err = spi_setcfg(fd_spi, 0, &cfg);
  if (cfg_err != EOK) {
    perror("接続に失敗しました\n");
    return 0;
  }
  // 1.レジスタ0x07に0x80を書き込む
  uint8_t kakikomi[2] = {0x87, 0x80};
  int add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }

  // 2.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  // 3.レジスタ0x07に0x00を書き込む
  kakikomi[0] = 0x87;
  kakikomi[1] = 0x00;
  add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }

  // 4.100msまつ
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);

  // I2Cパート
  // setconfig
  int fd = open("/dev/i2c1", O_RDWR);
  if (fd == -1) {
    perror("i2c1を開くのに失敗しました。");
    return 0;
  }

  // 1.レジスタ0xffに0x01を書き込む
  kakikomi[0] = 0xff;
  kakikomi[1] = 0x01;

  if (i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi)) == -1) {
    return -1;
  }

  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  //プロダクトIDの確認
  uint8_t product[1] = {0x0a};
  int product_err = i2c_read(fd, 0x30, product,sizeof(product));
  if (product_err == -1) {
    return -1;
  }
  product[0] = 0x0b;
  product_err = i2c_read(fd, 0x30, product,sizeof(product));
  if (product_err == -1) {
    return -1;
  }

  // 2.0x12に0x80をかきこむ
  kakikomi[0] = 0x12;
  kakikomi[1] = 0x80;

  //実際に書き込んでいく
  if (i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi)) == -1) {
    perror("書き込みに失敗しました。\n");
    return 0;
  }

  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  // 3.設定値の書き込み
  for (int i = 0; i < 206; i++) {
    kakikomi[0] = settei[i][0];
    kakikomi[1] = settei[i][1];
    /*printf("%d\n",kakikomi[0]);
    printf("%d\n",kakikomi[1]);*/
    add_er = i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi));
    if (add_er == -1) {
      perror("書き込みに失敗しましたX。\n");
      return 0;
    }
  }
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  //画像のピクセルについての設定値の書き込み
  for (int i = 0; i < 40; i++) {
    kakikomi[0] = gazou[i][0];
    kakikomi[1] = gazou[i][1];
    add_er = i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi));
    if (add_er == -1) {
      perror("書き込みに失敗しましたY。\n");
      return 0;
    }
  }

  //実際の撮影
  // SPIpart
  // 0x04に0x01を書き込む
  kakikomi[0] = 0x84;
  kakikomi[1] = 0x01;
  add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しましたa。\n");
    return 0;
  }
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  // 0x04に0x02を書き込む
  kakikomi[0] = 0x84;
  kakikomi[1] = 0x02;
  add_er = spi_write(fd_spi, 0, kakikomi, sizeof(kakikomi));
  if (add_er == -1) {
    perror("書き込みに失敗しましたb。\n");
    return 0;
  }
  //撮影完了か調べる
  // 0x11はdummy
  uint8_t yomikomi[2];
  yomikomi[0] = 0;
  yomikomi[1] = 0;
  while ((yomikomi[1] & 8) == 0) {
    printf("%d\n", yomikomi[0]);
    printf("%d\n", yomikomi[1]);
    printf("\n");
    uint8_t reg[2] = {0x41, 0x11};
    if (spi_xchange(fd_spi, 0, reg, yomikomi, sizeof(yomikomi)) < 0) {
      perror("読み込みに失敗しました\n");
      return 0;
    }
  }
  uint8_t reg_42[2];
  uint8_t reg_43[2];
  uint8_t reg_44[2];
  uint8_t reg[2] = {0x42, 0x11};
  if (spi_xchange(fd_spi, 0, reg, reg_42, sizeof(reg_42)) < 0) {
    perror("読み込みに失敗しました\n");
    return 0;
  }
  reg[0] = 0x43;
  if (spi_xchange(fd_spi, 0, reg, reg_43, sizeof(reg_43)) < 0) {
    perror("読み込みに失敗しました\n");
    return 0;
  }
  reg[0] = 0x44;
  if (spi_xchange(fd_spi, 0, reg, reg_44, sizeof(reg_44)) < 0) {
    perror("読み込みに失敗しました\n");
    return 0;
  }
  uint32_t reg42 = reg_42[1];
  uint32_t reg43 = reg_43[1];
  uint32_t reg44 = reg_44[1];
  reg43 = reg43 << 8;
  reg44 = reg44 << 16;
  reg44 += reg43 + reg42;
  if (reg44 > 8388608) {
    reg44 -= 8388608;
  }
  char gaso[reg44];
  reg[0] = 0x3c;
  if (spi_cmdread(fd_spi, 0, reg, sizeof(reg), gaso, sizeof(gaso)) < 0) {
    perror("読み込みに失敗しました\n");
    return 0;
  }
  //ファイル書き出し
  char *dest = "sample.jpeg";
  int dest_fp = open(dest, O_WRONLY | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
  uint8_t size = sizeof(gaso) - 1;
  const char *buf_sub = gaso + 1;
  while (size > 0) {
    int kakikomi = write(dest_fp, buf_sub, size);
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
    size -= kakikomi;
  }
  if(close(fd_spi)!=0){
    return -1;
  }

  return close(fd);
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

static int i2c_read(int fd, uint32_t addr, const void *yomikomi_reg, size_t
                    len) {
  int write_err = i2c_write(fd, addr, yomikomi_reg, len);
  if (write_err == -1) {
    return -1;
  }
  uint8_t yomikomi[1];
  i2c_recv_t *buf_rec = malloc(sizeof(i2c_recv_t) + sizeof(yomikomi));
  if (!buf_rec) {
    perror("読み込みに失敗しました\n");
    return 0;
  }
  buf_rec->slave.addr = addr;
  buf_rec->slave.fmt = I2C_ADDRFMT_7BIT;
  buf_rec->len = sizeof(yomikomi);
  buf_rec->stop = 1;  // 0 にするとトランザクション継続
  int err = devctl(fd, DCMD_I2C_RECV, buf_rec,
                   sizeof(i2c_recv_t) + sizeof(yomikomi), NULL);
  if (err != EOK) {
    perror("読み込みに失敗しました\n");
    return -1;
  }
  char *data = (char *)buf_rec + sizeof(i2c_recv_t);
  printf("%d\n", data[0]);
  free(buf_rec);
  return 0;
}
