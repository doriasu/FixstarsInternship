#include <hw/i2c.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define _XTAL_FREQ 4000000
//修正が必要
/*void kakikomu(int *fd, uint8_t *address, uint8_t *atai) {
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
}*/

int main(void) {
  //時間処理
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1e+8;

  int fd = spi_open("/dev/i2c0");
  // setconfig
  i2c_addr_t addr = {
      .addr = 0x30,
      .fmt = I2C_ADDRFMT_7BIT,
  };
  int cfg_err = devctl(fd, DCMD_I2C_SET_SLAVE_ADDR, &addr, sizeof(addr), NULL);

  if (cfg_err != EOK) {
    perror("接続に失敗しました\n");
    return 0;
  }
  uint32_t speed = I2C_SPEED_STANDARD;  // 100kbps
  cfg_err = devctl(fd, DCMD_I2C_SET_BUS_SPEED, &speed, sizeof(speed), NULL);
  if (cfg_err != EOK) {
    perror("接続に失敗しました\n");
    return 0;
  }

  // 1.レジスタ0xffに0x01を書き込む
  uint8_t kakikomi[3] = {0x60, 0xff, 0x01};
  for (int i = 0; i < 3; i++) {
    int add_er = write(fd, &kakikomi[i], sizeof(1));
    if (add_er == -1) {
      perror("書き込みに失敗しました。\n");
      return 0;
    }
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  }

  uint8_t yomikomi_reg[3] = {0x60, 0x0b, 0x61};
  uint8_t yomikomi[1];
  for (int i = 0; i < 3; i++) {
    int add_er = write(fd, yomikomi_reg, sizeof(yomikomi_reg));
    if (add_er == -1) {
      perror("書き込みに失敗しました。\n");
      return 0;
    }
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  }
  int read_er = read(fd, yomikomi, sizeof(yomikomi));
  printf("%d\n", yomikomi[0]);

  return 0;
}