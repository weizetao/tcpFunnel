/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_UPSTREAM_H
#define _XNET_UPSTREAM_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

int x_upstream_init(xtp_conn_t *pconn);
int x_upstream_handle(xtp_conn_listen_t *ln);
void x_upstream_final(xtp_conn_t *pconn);
void x_upstream_send_to_peer(xtp_conn_t *pconn);

#ifdef __cplusplus
}
#endif

#endif
