/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_NIO_H
#define _XNET_NIO_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

int x_recvn(int nSocket , void *pStr , unsigned int nLen);
int x_sendn(int nSocket , const void *pStr , unsigned int nLen);
int x_setNonBlock(int sockfd);
int x_socket_bind(int port);

#define x_ev_of_conn(ev_io_watch,member) \
    ((xtp_conn_t*)((char*)(ev_io_watch) - offsetof(xtp_conn_t, member)))

#define x_ev_of_conn_listen(ev_io_watch,member) \
    ((xtp_conn_listen_t*)((char*)(ev_io_watch) - offsetof(xtp_conn_listen_t, member)))


#ifdef __cplusplus
}
#endif


#endif

