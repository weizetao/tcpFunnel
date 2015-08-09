/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_CONNECTION_H
#define _XNET_CONNECTION_H
 
#include "xtp_config.h"
#include "xtp_core.h"
 
#ifdef __cplusplus
 extern "C" {
#endif

#define CONN_NUM_MAX (655350)
#define CONN_FREE_TIMEOUT 3.0
#define CONN_CONNECT_TIMEOUT 0.5

//typedef struct packet_buf {
//    STAILQ_ENTRY(packet_buf) pkt;
//    xtp_buf_t *buf;
//    char *data; //point to the data of body buf
//    int send_len;
//    int left_len;
//    int isclose;
//}packet_buf;


typedef enum xtp_conn_phase_s {
    CONN_NONE = 0,
    CONN_WAITING,
    CONN_WORKING,
    CONN_CONNECT,
    CONN_TIMEOUT,
    CONN_CLOSE,
    CONN_ERROR
} xtp_conn_phase_t;

struct xtp_conn_listen_s{
	xtp_srvconf_t			*srvcf;
    ngx_pool_t 				*p;
    xtp_buf_pool_t    		*bp;
    char    				ip[16];
    int     				port;
    int     				fd;

    int 						c_count;

    TAILQ_HEAD(, xtp_conn_s) 	cq_waiting;	//connect queue
    int 						cq_waiting_num;
    int							working_num;

    struct ev_loop 			*loop;
    ev_io   				e_listen;
    ev_async				e_sig;

    int     				timeout;
};

struct xtp_conn_s {
    char    ip[16];
    int     port;
    int     fd;
    xtp_conn_phase_t   status;

    void    (*cleanup)(void*); //cleanup the parser/user/ptr

    xtp_conn_listen_t *ln;
    
    ev_io   e_recv;
    ev_io   e_send;
    ev_timer    e_timer;
    
    /* Recv & Send buff */
    STAILQ_HEAD(, xtp_buf_s)		pipe1;
    STAILQ_HEAD(, xtp_buf_s)		pipe2;

    xtp_upstream_conf_t 	*upcf;
    int						upc_fd;
    ev_io					upc_e_recv;		//upstream connection
    ev_io					upc_e_send;
    ev_io					upc_e_connected;
    ev_timer				upc_e_timer;
    xtp_conn_phase_t		upc_status;

    TAILQ_ENTRY(xtp_conn_s) entries;
};

#ifdef __cplusplus
 }
#endif
 
#endif


