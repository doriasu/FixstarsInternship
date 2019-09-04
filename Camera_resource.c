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
int io_write_kaizoudo(resmgr_context_t *ctp, io_write_t *msg,
                      RESMGR_OCB_T *ocb);
int my_func(message_context_t *ctp, int code, unsigned flags, void *handle);
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int satsuei_shoki = 0;

int main(void) {
  satsuei_flag = 0;
  //ディスパッチ構造体の作成と各種変数の定義
  //撮影用
  dispatch_t *dpp;
  dispatch_context_t *ctp, *new_ctp;
  iofunc_attr_t my_attr_t;
  resmgr_connect_funcs_t my_connect_functions;
  resmgr_io_funcs_t my_io_functions;
  if ((dpp = dispatch_create()) == NULL) {
    perror("ディスパッチ構造体の生成に失敗しました。\n");
    return 0;
  }
  //解像度変更用
  iofunc_attr_t my_attr_t_kaizoudo;
  resmgr_connect_funcs_t my_connect_functions_kaizoudo;
  resmgr_io_funcs_t my_io_functions_kaizoudo;

  // resmgr_connect_funcs_t構造体とresmgr_io_funcs_t構造体の初期化

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &my_connect_functions,
                   _RESMGR_IO_NFUNCS, &my_io_functions);
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &my_connect_functions_kaizoudo,
                   _RESMGR_IO_NFUNCS, &my_io_functions_kaizoudo);
  // open,read,write,devctlの設定
  //関数ポインタを渡す
  my_connect_functions.open = io_open;
  my_io_functions.read = io_read;
  my_io_functions.write = io_write;
  my_io_functions.devctl = io_devctl;

  my_io_functions_kaizoudo.write = io_write_kaizoudo;

  // iofunc_attr_t構造体の初期化

  iofunc_attr_init(&my_attr_t, S_IFCHR | 0666, NULL, NULL);
  iofunc_attr_init(&my_attr_t_kaizoudo, S_IFCHR | 0666, NULL, NULL);
  /// dev/mynullのアタッチ
  int id = resmgr_attach(dpp, NULL, "/dev/Camera/satsuei", _FTYPE_ANY, 0,
                         &my_connect_functions, &my_io_functions, &my_attr_t);
  if (id == -1) {
    perror("/dev/mynullのアタッチに失敗しました。\n");
    return 0;
  }

  int id_kaizoudo =
      resmgr_attach(dpp, NULL, "/dev/Camera/kaizoudo", _FTYPE_ANY, 0,
                    &my_connect_functions_kaizoudo, &my_io_functions_kaizoudo,
                    &my_attr_t_kaizoudo);
  if (id_kaizoudo == -1) {
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
  close(fd);

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
  close(fd_spi);
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
  //撮影済みかどうかで分岐
  if (satsuei_flag) {
    if (gazou_size < msg->i.nbytes) {
      yomikomi = gazou_size;

    } else {
      yomikomi = msg->i.nbytes;
    }
    uint8_t burst[1] = {0x3c};
    //読み出し領域の確保
    char *gaso = malloc(sizeof(char) * (yomikomi));
    if (gaso == NULL) {
      perror("写真用メモリの確保に失敗しました。\n");
    }
    if ((real_size = spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso,
                                 yomikomi)) < 0) {
      perror("書き込みに失敗しました\n");
      return EBADF;
    }
    //撮影したばかりの時はburstmode用の1byte読み飛ばしが必要
    if (satsuei_shoki) {
      gaso++;
      real_size--;
      satsuei_shoki = 0;
    }
    // readのデータを返すときにはcto->iov[0]に対しポインタの受け渡しとサイズの容量の追記が必要
    // SETIOVでも代用可能
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
    yomikomi++;
    char *gaso = malloc(sizeof(char) * yomikomi);
    if (gaso == NULL) {
      perror("写真用メモリの確保に失敗しました。\n");
    }
    if ((real_size = spi_cmdread(fd_spi, 0, burst, sizeof(burst), gaso,
                                 (uint32_t)yomikomi)) < 0) {
      int tmp_err = errno;
      perror("書き込みに失敗しましたe\n");
      return tmp_err;
    }
    //上の分に同じ
    gaso++;
    real_size--;
    satsuei_shoki = 0;
    ctp->iov[0].iov_base = gaso;
    ctp->iov[0].iov_len = real_size;
    gazou_size -= real_size;
    satsuei_flag = 1;
  }

  // ctpに対し読んだ容量を書き込むことが必要
  _IO_SET_READ_NBYTES(ctp, real_size);

  if (real_size > 0) { /* mark access time for update */
    ((struct _iofunc_ocb *)ocb)->attr->flags |= IOFUNC_ATTR_ATIME;
  }
  printf("readしたよ\n");
  //ここを1にしないとreadの内容を返すことができない(数を表す)
  return _RESMGR_NPARTS(1);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
  return ENOSYS;
}
int io_write_kaizoudo(resmgr_context_t *ctp, io_write_t *msg,
                      RESMGR_OCB_T *ocb) {
  int status;
  if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
    return status;
  }
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
    return ENOSYS;
  }

  int yomikomi = msg->i.nbytes;
  //yomikomiには文字列の他に何らかのものが付いているので-1する\0とかかな
  yomikomi--;
  const char *buf_sub = (char *)msg + sizeof(io_write_t);
  if (strncmp(buf_sub, "160X120", yomikomi) == 0) {
    kaizoudo = gazou_160;
    kaizoudo_size = sizeof(gazou_160);
    kaizoudo_now=1;
  } else if (strncmp(buf_sub, "176X144", yomikomi) == 0) {
    kaizoudo = gazou_176;
    kaizoudo_size = sizeof(gazou_176);
    kaizoudo_now=2;
  } else if (strncmp(buf_sub, "320X240", yomikomi) == 0) {
    kaizoudo = gazou_320;
    kaizoudo_size = sizeof(gazou_320);
    kaizoudo_now=3;
  } else if (strncmp(buf_sub, "352X288", yomikomi) == 0) {
    kaizoudo = gazou_352;
    kaizoudo_size = sizeof(gazou_352);
    kaizoudo_now=4;
  } else if (strncmp(buf_sub, "640X480", yomikomi) == 0) {
    kaizoudo = gazou_640;
    kaizoudo_size = sizeof(gazou_640);
    kaizoudo_now=5;
  } else if (strncmp(buf_sub, "800X600", yomikomi) == 0) {
    kaizoudo = gazou_800;
    kaizoudo_size = sizeof(gazou_800);
    kaizoudo_now=6;
  } else if (strncmp(buf_sub, "1024X768", yomikomi) == 0) {
    kaizoudo = gazou_1024;
    kaizoudo_size = sizeof(gazou_1024);
    kaizoudo_now=7;
  } else if (strncmp(buf_sub, "1280X1024", yomikomi) == 0) {
    kaizoudo = gazou_1280;
    kaizoudo_size = sizeof(gazou_1280);
    kaizoudo_now=8;
  } else if (strncmp(buf_sub, "1600X1200", yomikomi) == 0) {
    kaizoudo = gazou_1600;
    kaizoudo_size = sizeof(gazou_1600);
    kaizoudo_now=9;
  } else {
    printf("そのような解像度は存在しません。\n");
  
    return ENOSYS;
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
  close(fd);
  //_IO_SET_WRITE_NBYTESは正しい容量を書き込まないと無限ループにはまるので戻す
  yomikomi++;
  _IO_SET_WRITE_NBYTES(ctp, yomikomi);

  if (yomikomi > 0) { /* mark times for update */
    ((struct _iofunc_ocb *)ocb)->attr->flags |=
        IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
  }
  printf("現在の解像度は%s\n", namae[kaizoudo_now - 1]);

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
  status = 0;
  int nbytes = 0;
  union {
    data_t data;
    uint32_t data32;
  } * rx_data;
  //データを返す場合用
  rx_data = _DEVCTL_DATA(msg->i);
  switch (msg->i.dcmd) {
    //解像度の変更用(header.hに定数が割り当ててあるのでそれを使う)
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
      close(fd);

      kaizoudo_now = *(int *)_DEVCTL_DATA(msg->i);
      printf("現在の解像度は%s\n", namae[kaizoudo_now - 1]);

      break;
    //現在の解像度の取得用
    case DCMD_CAMERA_GETRES:
      printf("%d\n", kaizoudo_now);
      //解像度のコードの受け渡し
      rx_data->data32 = kaizoudo_now;
      nbytes = sizeof(rx_data->data32);
      break;
    //再撮影
    case DCMD_CAMERA_SHOT:
      satsuei_flag = 1;
      gazou_size = satsuei(fd_spi);
      printf("撮影したのでread読んでください\n");
      satsuei_shoki = 1;
      break;
    //まだ受け渡していない容量の取得
    case DCMD_CAMERA_GETSIZE:
      rx_data->data32 = gazou_size;
      nbytes = sizeof(rx_data->data32);
      break;
  }
  memset(&msg->o, 0, sizeof(msg->o));
  //書き込んだ容量の登録が必要
  msg->o.nbytes = nbytes;
  printf("devctlしたよ\n");
  //こういうものらしい
  return (_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));
}
