/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_ACCESS_H
#define _XNET_ACCESS_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void x_access_new(xtp_server_t *srv);
int x_access_send_to_client(xtp_conn_t *pconn);

#ifdef __cplusplus
}
#endif




#endif

