#include <errno.h>
#include<stdio.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>

int io_open(resmgr_context_t *ctp, io_open_t *msg,RESMGR_HANDLE_T *handle,void*extra);
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int main(void) {
  //ディスパッチ構造体の作成と各種変数の定義
  dispatch_t *dpp;
  dispatch_context_t *ctp;
  resmgr_context_t ctp_res;
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
  // open,read,writeの設定
  //関数ポインタを渡す
  my_connect_functions.open = io_open;
  my_io_functions.read = io_read;
  my_io_functions.write = io_write;

  // iofunc_attr_t構造体の初期化
  
  iofunc_attr_init(&my_attr_t, S_IFCHR | 0666, NULL, NULL);
  ///dev/mynullのアタッチ
  int id = resmgr_attach(dpp, NULL, "/dev/mynull", _FTYPE_ANY, 0,
                         &my_connect_functions, &my_io_functions, &my_attr_t);
  ctp = dispatch_context_alloc(dpp);
  while (1) {
    ctp = dispatch_block(ctp);
    dispatch_handler(ctp);
  }
}
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,void*extra) {
  return iofunc_open_default(ctp,msg,
                             handle, extra);
}
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
    int status;
    if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
        return status;
    }
    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
        return ENOSYS;
    }

    _IO_SET_READ_NBYTES(ctp, 0); /* 0 bytes successfully read */

    if (msg->i.nbytes > 0) { /* mark access time for update */
        *ocb.attr->flags |= IOFUNC_ATTR_ATIME;
    }

    return _RESMGR_NPARTS(0);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
    int status;
    if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
        return status;
    }
    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
        return ENOSYS;
    }

    _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);

    if (msg->i.nbytes > 0) { /* mark times for update */
        *ocb.attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
    }

    return _RESMGR_NPARTS(0);
}
