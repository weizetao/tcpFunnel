/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"


void x_buf_init(xtp_buf_pool_t *bp, int max, int size)
{
    int i = 0;
    bp->n = 0;
    bp->max = 0;
    bp->offset = sizeof(xtp_buf_t);
    bp->size = size;//
    bp->head = NULL;
    for (i=0; i<max ; i++)
    {
        xtp_buf_t *buf = (xtp_buf_t*)malloc(bp->size);
        if (NULL == buf)
            continue;
        memset(buf, 0, bp->size);

        buf->bp = bp;
        buf->start = (u_char*)buf + bp->offset;
        buf->end = (u_char*)buf + bp->size - 1;

        buf->next = bp->head;
        bp->head = buf;

        bp->n++;
        bp->max++;
    }
    //    pthread_mutex_init(&bp->mutex,NULL);
}

void x_buf_uninit(xtp_buf_pool_t *bp)
{
    while(bp->head != NULL)
    {
        xtp_buf_t *tmp = bp->head;
        bp->head = bp->head->next;
        free(tmp);
    }
    //    pthread_mutex_destroy(&bp->mutex);
}
void x_buf_ralloc(xtp_buf_pool_t *bp, int num)
{
    int i = 0;
    xtp_buf_t *buf = NULL;
    bp->max = bp->max?bp->max:1;
    num = num?num:bp->max;
    for (i=0; i<num; i++)
    {
        buf = (xtp_buf_t*)malloc(bp->size);
        if (NULL == buf)
            continue;

        memset(buf, 0, bp->size);
        buf->bp = bp;
        buf->start = (u_char*)buf + bp->offset;
        buf->end = (u_char*)buf + bp->size - 1;
        buf->pos = buf->start;
        buf->last = buf->start;

        buf->next = bp->head;
        bp->head = buf;

        bp->n++;
        bp->max++;
    }
}

xtp_buf_t * x_buf_alloc(xtp_buf_pool_t *bp)
{
    x_log_debug("x_buf_alloc n=%d",bp->n);
    if ( bp->head == NULL){
        x_buf_ralloc(bp, 0);
        x_log_warn("after x_buf_alloc num=%d",bp->max);
        return x_buf_alloc(bp);
    }
    xtp_buf_t *buf = bp->head;
    bp->head = buf->next;
    buf->next = NULL;
    bp->n--;

    buf->start = (u_char*)buf + bp->offset;
    buf->end = (u_char*)buf + bp->size - 1;
    buf->pos = buf->start;
    buf->last = buf->start;

    return buf;
}

/* adjust to big buf */
xtp_buf_t* x_buf_adjust(xtp_buf_t *buf, int size)
{
    if(size <= buf->size)
        return buf;
    xtp_buf_pool_t *bp = (xtp_buf_pool_t*)buf->bp;
    int newsize = (((size + bp->offset) / BUFSIZ) + 1) * BUFSIZ;
    xtp_buf_t *tmp = (xtp_buf_t*)realloc(buf, newsize);
    if(!tmp)
        return NULL;
    tmp->size = newsize - bp->offset;
    tmp->start = (u_char*)tmp + bp->offset;
    tmp->end = (u_char*)tmp + newsize - 1;
    tmp->pos = tmp->start;
    tmp->last = tmp->start;
    return tmp;
}

void x_buf_free(xtp_buf_t *buf)
{
    if(!buf || !buf->bp)
        return;
    xtp_buf_pool_t *bp = (xtp_buf_pool_t*)buf->bp;
    //    pthread_mutex_lock(&bp->mutex);
    if(buf->size > bp->size){
        free(buf);
        bp->max--;
    }else{
        buf->next = bp->head;
        bp->head = buf;
        bp->n++;
    }
    //    pthread_mutex_unlock(&bp->mutex);
}


