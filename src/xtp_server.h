/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_SERVER_H
#define _XNET_SERVER_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xtp_server_s {
	xtp_srvconf_t 				*srvconf;
	xtp_conn_listen_t 			*listen;
	xtp_thread_t				*thr;
};

xtp_server_t *x_server_new();

void x_server_destroy(xtp_server_t *srv);

#ifdef __cplusplus
}
#endif

#endif

