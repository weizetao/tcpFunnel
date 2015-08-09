/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_BUFPOOL_H
#define _XNET_BUFPOOL_H
 
#include "xtp_config.h"
#include "xtp_core.h"
 
#ifdef __cplusplus
 extern "C" {
#endif

typedef enum{
    BUFF_NONE = 0,
    BUFF_RECV_HEADER,
    BUFF_RECV_BODY,
    BUFF_RECVED,
    BUFF_COMPLETE,
    BUFF_SEND,
    BUFF_DONE,
    BUFF_CLOSE,
    BUFF_ERROR
}buff_phase;

#define NODE_BUFSIZ (BUFSIZ)
#define BUFF_HEADER_SIZE (sizeof(int))


struct xtp_buf_s {
	xtp_buf_t 		*next;
    void    			*bp;     //the buf pool

	u_char 				*start;  //point to the real data buff
	u_char 				*end;  	 //
    u_char  			*pos;
    u_char  			*last;
	int 				size;   //the size of buff

	unsigned         	last_buf:1;

	STAILQ_ENTRY(xtp_buf_s) entries;
} ;


struct xtp_buf_pool_s {
    int     			n;      //remaining n
    int     			max;    //all
    int    				size;   //the size of every buf
    size_t  			offset; //the xtp_buf_t size
//    pthread_mutex_t mutex;
    xtp_buf_t 		*head;
};

void x_buf_init(xtp_buf_pool_t *bp, int max, int size);
void x_buf_uninit(xtp_buf_pool_t *bp);
xtp_buf_t * x_buf_alloc(xtp_buf_pool_t *bp);

void x_buf_ralloc(xtp_buf_pool_t *bp, int num);

xtp_buf_t* x_buf_adjust(xtp_buf_t *buf, int size);
void x_buf_free(xtp_buf_t *buf);


#ifdef __cplusplus
 }
#endif
 
#endif



