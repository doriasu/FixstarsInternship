#include <errno.h>
#include <fcntl.h>
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
char file_path[256];
int main(void) {
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
  int id = resmgr_attach(dpp, NULL, "/dev/mynull", _FTYPE_ANY, 0,
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
  return iofunc_open_default(ctp, msg, handle, extra);
}
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
  int status;
  if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
    return status;
  }
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
    return ENOSYS;
  }

  _IO_SET_READ_NBYTES(ctp, 0); /* 0 bytes successfully read */

  if (msg->i.nbytes > 0) { /* mark access time for update */
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
      return _RESMGR_NPARTS(-1);
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

          return _RESMGR_NPARTS(-1);
        }
      }
      buf_sub += kakikomi;
      yomikomi -= kakikomi;
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
      snprintf(file_path, sizeof(file_path), _DEVCTL_DATA(msg->i));
      break;
  }
  printf("devctlしたよ\n");
  return _RESMGR_NPARTS(0);
}