/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_CORE_H
#define _XNET_CORE_H

typedef struct xtp_srvconf_s   				xtp_srvconf_t;
typedef struct xtp_server_s 					xtp_server_t;

typedef struct ngx_pool_s        				ngx_pool_t;
typedef struct xtp_buf_s 				    	xtp_buf_t;
typedef struct xtp_frame_s 				    	xtp_frame_t;
typedef struct xtp_buf_pool_s   				xtp_buf_pool_t;
typedef struct xtp_conn_listen_s 				xtp_conn_listen_t;
typedef struct xtp_conn_s 						xtp_conn_t;
typedef struct xtp_thread_s 					xtp_thread_t;

struct xtp_conn_s;

typedef enum{
        XNET_PASS,
        XNET_INGNORE,
        XNET_HANDLED
} xtp_mod_return_e;

typedef enum{
        XNET_NDX_DEFAULT,
        XNET_NDX_FIRST,
        XNET_NDX_MIDDLE,
        XNET_NDX_LAST
} xtp_mod_ndx_e;


#include "display.h"
#include "log.h"
#include "zmalloc.h"

//#include "ngx_alloc.h"
//#include "ngx_palloc.h"
#include "queue.h"
#include "xtp_bufpool.h"

#include "iksemel.h"

#include "xtp_conf.h"
#include "xtp_server.h"

#include "xtp_nio.h"
#include "xtp_connection.h"
#include "xtp_access.h"
#include "xtp_thread.h"
#include "xtp_upstream.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char Bytes;
typedef unsigned char u_char;



#ifdef __cplusplus
}
#endif

#endif
