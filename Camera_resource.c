#include <errno.h>
#include <fcntl.h>
#include <hw/i2c.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "header.h"
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
            void *extra);
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int my_func(message_context_t *ctp, int code, unsigned flags, void *handle);
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
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
char kaizoudo_now[20];
int main(void) {
  satsuei_flag = 0;
  //ディスパッチ構造体の作成と各種変数の定義
  dispatch_t *dpp;
  dispatch_context_t *ctp, *new_ctp;
  iofunc_attr_t my_attr_t;
  resmgr_connect_funcs_t my_connect_functions;
  resmgr_io_funcs_t my_io_functions;
  if ((dpp = dispatch_create()) == NULL) {
    perror("ディスパッチ構造体の生成に失敗しました。\n");
    return 0;
  }
  // resmgr_connect_funcs_t構造体とresmgr_io_funcs_t構造体の初期化

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &my_connect_functions,
                   _RESMGR_IO_NFUNCS, &my_io_functions);
  // open,read,write,devctlの設定
  //関数ポインタを渡す
  my_connect_functions.open = io_open;
  my_io_functions.read = io_read;
  my_io_functions.write = io_write;
  my_io_functions.devctl = io_devctl;

  // iofunc_attr_t構造体の初期化

  iofunc_attr_init(&my_attr_t, S_IFCHR | 0666, NULL, NULL);
  /// dev/mynullのアタッチ
  int id = resmgr_attach(dpp, NULL, "/dev/Camera", _FTYPE_ANY, 0,
                         &my_connect_functions, &my_io_functions, &my_attr_t);
  if (id == -1) {
    perror("/dev/mynullのアタッチに失敗しました。\n");
    return 0;
  }
  ctp = dispatch_context_alloc(dpp);
  //パルスの処理
  int code = pulse_attach(dpp, 0, PULSE_CODE, &my_func, NULL);
  if (code == -1) {
    perror("パルスの処理に失敗しました。");
    return 0;
  }
  //カメラのセットアップ
  fd_spi = spi_open("/dev/spi0");
  if (fd_spi == -1) {
    perror("spiファイルを開くのに失敗しました。\n");
    return 0;
  }
  int fd = open("/dev/i2c1", O_RDWR);
  if (fd == -1) {
    perror("i2c1を開くのに失敗しました。");
    return 0;
  }
  if (Camera_setup(fd) == -1) {
    printf("カメラの初期化に失敗しました。アプリケーションを終了します。\n");
    return 0;
  }

  //メッセージの待受と処理
  while (1) {
    new_ctp = dispatch_block(ctp);
    if (new_ctp) {
      ctp = new_ctp;
    } else {
      perror("メッセージの待受に失敗しました。\n");
      return 0;
    }
    int handle_err = dispatch_handler(ctp);
    if (handle_err == -1) {
      perror("メッセージの処理に失敗しました。\n");
      return 0;
    }
  }
  return 0;
}
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
            void *extra) {
  printf("openしたよ\n");
  satsuei_flag = 0;
  return iofunc_open_default(ctp, msg, handle, extra);
}
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
  int status;
  uint32_t yomikomi;
  if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
    return status;
  }
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
    return ENOSYS;
  }
  if (satsuei_flag) {
    if (gazou_size < msg->i.nbytes) {
      yomikomi = gazou_size;

    } else {
      yomikomi = msg->i.nbytes;
    }
    char gaso[yomikomi];
    uint8_t burst[1] = {0x3c};
    if ((spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso, sizeof(gaso))) <
        0) {
      perror("読み込みに失敗しましたe\n");
      return EBADF;
    }
    gazou_size -= yomikomi;
    char *buf_sub = (char *)msg + sizeof(io_read_t);
    strncpy(buf_sub, gaso, yomikomi);
    if (gazou_size == 0) {
      satsuei_flag = 0;
      return 0;
    } else {
      satsuei_flag = 1;
      return 1;
    }

  } else {
    //撮影していない場合なのでとりあえず撮影する
    gazou_size = satsuei(fd_spi);
    //実際の読み込み操作
    yomikomi = msg->i.nbytes;
    char gaso[yomikomi];
    uint8_t burst[1] = {0x3c};
    if ((spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso, sizeof(gaso))) <
        0) {
      perror("読み込みに失敗しましたe\n");
      return EBADF;
    }
    gazou_size -= yomikomi;
    //かきだす
    char *buf_sub = (char *)msg + sizeof(io_read_t);
    strncpy(buf_sub, gaso, yomikomi);
    if (gazou_size == 0) {
      satsuei_flag = 0;
      return 0;
    } else {
      satsuei_flag = 1;
    }
  }

  _IO_SET_READ_NBYTES(ctp, yomikomi); /* 0 bytes successfully read */

  if (yomikomi > 0) { /* mark access time for update */
    ((struct _iofunc_ocb *)ocb)->attr->flags |= IOFUNC_ATTR_ATIME;
  }
  printf("readしたよ\n");

  return _RESMGR_NPARTS(0);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
  int status;
  if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
    return status;
  }
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
    return ENOSYS;
  }

  //書き込み処理
  if (strlen(file_path) > 0) {
    int dest_fp =
        open(file_path, O_WRONLY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
    if (dest_fp == -1) {
      perror("書き込みファイルのオープンでエラーが発生しました。\n");
      return 0;
    }
    int yomikomi = msg->i.nbytes;
    const char *buf_sub = (char *)msg + sizeof(io_write_t);
    int sum = 0;

    while (yomikomi > 0) {
      int kakikomi = write(dest_fp, buf_sub, yomikomi);
      sum += kakikomi;
      //エラー処理
      if (kakikomi == -1) {
        if (errno == EINTR) {
        } else {
          perror("ファイルの書き込みに失敗しました。\n");

          break;
        }
      }
      buf_sub += kakikomi;
      yomikomi -= kakikomi;
    }
    if (sum <= 0) {
      sum = 1;
    }
    _IO_SET_WRITE_NBYTES(ctp, sum);

    if (sum > 0) { /* mark times for update */
      ((struct _iofunc_ocb *)ocb)->attr->flags |=
          IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
    }
    close(dest_fp);
  } else {
    //書いたふり(ファイルパスが指定されていない時のため)
    _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);

    if (msg->i.nbytes > 0) { /* mark times for update */
      ((struct _iofunc_ocb *)ocb)->attr->flags |=
          IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
    }
  }

  printf("writeしたよ\n");

  return _RESMGR_NPARTS(0);
}
int my_func(message_context_t *ctp, int code, unsigned flags, void *handle) {
  if (code == PULSE_CODE) {
    printf("指定されたパルスを取得したのでアプリケーションを終了します\n");
    exit(0);
  }
  return 0;
}
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {
  int status = iofunc_devctl_default(ctp, msg, ocb);
  if (status != _RESMGR_DEFAULT) {
    return status;
  }
  switch (msg->i.dcmd) {
    case DCMD_MYNULL_KAKIKOMI:
      //ファイル名の書き込み
      // printf("%s\n", (char *)_DEVCTL_DATA(msg->i));
      sprintf(file_path, _DEVCTL_DATA(msg->i));
      break;
    case DCMD_CAMERA_SETRES:
      if (strcmp(_DEVCTL_DATA(msg->i), "160*120") == 0) {
        kaizoudo = gazou_160;
        kaizoudo_size = sizeof(gazou_160);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "176*144") == 0) {
        kaizoudo = gazou_176;
        kaizoudo_size = sizeof(gazou_176);
      } else if (strcmp(_DEVCTL_DATA(msg->i), "320*240") == 0) {
        kaizoudo = gazou_320;
        kaizoudo_size = sizeof(gazou_320);
      } else if (strcmp(_DEVCTL_DATA(msg->i), "352*288") == 0) {
        kaizoudo = gazou_352;
        kaizoudo_size = sizeof(gazou_352);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "640*480") == 0) {
        kaizoudo = gazou_640;
        kaizoudo_size = sizeof(gazou_640);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "800*600") == 0) {
        kaizoudo = gazou_800;
        kaizoudo_size = sizeof(gazou_800);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "1024*768") == 0) {
        kaizoudo = gazou_1024;
        kaizoudo_size = sizeof(gazou_1024);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "1280*1024") == 0) {
        kaizoudo = gazou_1280;
        kaizoudo_size = sizeof(gazou_1280);

      } else if (strcmp(_DEVCTL_DATA(msg->i), "1600*1200") == 0) {
        kaizoudo = gazou_1600;
        kaizoudo_size = sizeof(gazou_1600);

      } else {
        printf("その解像度は存在しません。\n");
        return 0;
      }
      strncpy(kaizoudo_now, _DEVCTL_DATA(msg->i), sizeof(kaizoudo_now));

      break;
    case DCMD_CAMERA_GETRES:
      strncpy(((char*)msg)+(sizeof(struct _io_devctl)),kaizoudo_now,sizeof(kaizoudo_now));
      break;
    case DCMD_CAMERA_SHOT:
      satsuei_flag = 0;
      printf("readをよんでください\n");
      break;
    case DCMD_CAMERA_GETSIZE:
      *(((uint32_t*)msg)+(sizeof(struct _io_devctl)))=gazou_size;
      break;
  }
  // sprintf(file_path, "/tmp/abc.txt");
  printf("devctlしたよ\n");
  return _RESMGR_NPARTS(0);
}

//以下カメラに関する関数
int Camera_setup(int fd) {
  //解像度640*480
  kaizoudo_size = 0;
  kaizoudo = gazou_640;
  kaizoudo_size = sizeof(gazou_640);
  strncpy(kaizoudo_now, "640*480", sizeof(kaizoudo_now));
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
