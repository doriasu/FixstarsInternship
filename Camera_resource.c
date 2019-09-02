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
#include "camera.h"
#include "header.h"
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
            void *extra);
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int my_func(message_context_t *ctp, int code, unsigned flags, void *handle);
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

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
  if (Camera_setup(fd, 1) == -1) {
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
  if (gazou_size == 0 && satsuei_flag == 1) {
    satsuei_flag = 0;
    return _RESMGR_NPARTS(0);
  }
  int real_size;
  printf("read始めるよ\n");
  if (satsuei_flag) {
    if (gazou_size < msg->i.nbytes) {
      yomikomi = gazou_size;

    } else {
      yomikomi = msg->i.nbytes;
    }
    uint8_t burst[1] = {0x3c};
    char gaso[yomikomi];
    char *buf_sub = (char *)msg + sizeof(io_read_t);
    if ((real_size = spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso,
                                 yomikomi)) < 0) {
      perror("書き込みに失敗しました\n");
      return EBADF;
    }
    ctp->iov[0].iov_base = gaso;
    ctp->iov[0].iov_len = real_size;

    gazou_size -= real_size;

  } else {
    //撮影していない場合なのでとりあえず撮影する
    gazou_size = satsuei(fd_spi);
    printf("撮影したよ\n");
    //実際の読み込み操作
    yomikomi = min((uint32_t)(msg->i.nbytes), gazou_size);
    uint8_t burst[1] = {0x3c};
    char gaso[yomikomi];
    char *buf_sub = (char *)msg + sizeof(io_read_t);
    if ((real_size = spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso,
                                 (uint32_t)yomikomi)) < 0) {
      int tmp_err = errno;
      perror("書き込みに失敗しましたe\n");
      return tmp_err;
    }
    ctp->iov[0].iov_base = gaso;
    ctp->iov[0].iov_len = real_size;
    gazou_size -= real_size;
    satsuei_flag = 1;
  }

  _IO_SET_READ_NBYTES(ctp, real_size);

  if (real_size > 0) { /* mark access time for update */
    ((struct _iofunc_ocb *)ocb)->attr->flags |= IOFUNC_ATTR_ATIME;
  }
  printf("readしたよ\n");

  return _RESMGR_NPARTS(1);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
  return ENOSYS;
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
      if (*(int *)_DEVCTL_DATA(msg->i) == 1) {
        kaizoudo = gazou_160;
        kaizoudo_size = sizeof(gazou_160);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 2) {
        kaizoudo = gazou_176;
        kaizoudo_size = sizeof(gazou_176);
      } else if (*(int *)_DEVCTL_DATA(msg->i) == 3) {
        kaizoudo = gazou_320;
        kaizoudo_size = sizeof(gazou_320);
      } else if (*(int *)_DEVCTL_DATA(msg->i) == 4) {
        kaizoudo = gazou_352;
        kaizoudo_size = sizeof(gazou_352);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 5) {
        kaizoudo = gazou_640;
        kaizoudo_size = sizeof(gazou_640);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 6) {
        kaizoudo = gazou_800;
        kaizoudo_size = sizeof(gazou_800);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 7) {
        kaizoudo = gazou_1024;
        kaizoudo_size = sizeof(gazou_1024);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 8) {
        kaizoudo = gazou_1280;
        kaizoudo_size = sizeof(gazou_1280);

      } else if (*(int *)_DEVCTL_DATA(msg->i) == 9) {
        kaizoudo = gazou_1600;
        kaizoudo_size = sizeof(gazou_1600);

      } else {
        printf("その解像度は存在しません。\n");
        return 0;
      }
      //カメラ再設定
      int fd = open("/dev/i2c1", O_RDWR);
      if (fd == -1) {
        perror("i2c1を開くのに失敗しました。");
        return 0;
      }
      uint8_t kakikomi[2];
      // 画像のピクセルについての設定値の書き込み
      for (int i = 0; i < kaizoudo_size / 2; i++) {
        kakikomi[0] = kaizoudo[i][0];
        kakikomi[1] = kaizoudo[i][1];
        int add_er = i2c_write(fd, 0x30, kakikomi, sizeof(kakikomi));
        if (add_er == -1) {
          perror("書き込みに失敗しました。\n");
          return -1;
        }
      }

      kaizoudo_now = *(int *)_DEVCTL_DATA(msg->i);
      printf("現在の解像度は%s\n", namae[kaizoudo_now - 1]);

      break;
    case DCMD_CAMERA_GETRES:
      memcpy(((char *)msg) + (sizeof(struct _io_devctl)), (int *)kaizoudo_now,
             sizeof(kaizoudo_now));
      break;
    case DCMD_CAMERA_SHOT:
      satsuei_flag = 0;
      printf("readをよんでください\n");
      break;
    case DCMD_CAMERA_GETSIZE:
      *(((uint32_t *)msg) + (sizeof(struct _io_devctl))) = gazou_size;
      break;
  }
  // sprintf(file_path, "/tmp/abc.txt");
  printf("devctlしたよ\n");
  return _RESMGR_NPARTS(0);
}
