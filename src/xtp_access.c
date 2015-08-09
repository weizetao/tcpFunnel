/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"


static void x_send_cb(EV_P_ ev_io *w, int revents);
static void x_recv_cb(EV_P_ ev_io *w, int revents);
static void x_timeout_cb(EV_P_ ev_timer *w, int revents);


/* close the connection 
 * flag:1-send notify to backend
 *      0-no send notify*/
void x_event_close(xtp_conn_t *pconn, int flag)
{
    if(pconn->status != CONN_CLOSE){
        x_log_debug("event close!");
        if(pconn->upc_status == CONN_NONE){
            x_log_info("[%d-%d] %s --> None ", pconn->status, pconn->upc_status, pconn->ip);
        }else{
            x_log_info("[%d-%d] %s --> %s ", pconn->status, pconn->upc_status, pconn->ip, pconn->upcf->ip);
        }
        x_upstream_final(pconn);
        ev_io_stop(pconn->ln->loop, &pconn->e_recv);
        ev_io_stop(pconn->ln->loop, &pconn->e_send);
        close(pconn->fd);
        if(pconn->status == CONN_WAITING){
            TAILQ_REMOVE(&pconn->ln->cq_waiting, pconn, entries);
            pconn->ln->cq_waiting_num--;
        }
        pconn->e_timer.repeat = CONN_FREE_TIMEOUT;
        ev_timer_again(pconn->ln->loop, &pconn->e_timer);
        pconn->status = CONN_CLOSE;
        pconn->ln->c_count --;
    }
}

static void x_recv_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, e_recv);
    xtp_buf_t *buf = NULL;
    int len = 0;

    buf = STAILQ_LAST(&pconn->pipe1, xtp_buf_s, entries);
    if(!buf){
        buf = x_buf_alloc(pconn->ln->bp);
        STAILQ_INSERT_TAIL(&pconn->pipe1, buf, entries);
    }
recv2buf:

    while(buf->last != buf->end){
        len = x_recvn(pconn->fd, buf->last, buf->end - buf->last);
        x_log_debug("access recv len=%d", len);
        if(len < 0){
            x_event_close(pconn, 1);
            return;
        }
        else if (len == 0){
            break;
        }else{
            buf->last += len;
            if(buf->end == buf->last){
                buf = x_buf_alloc(pconn->ln->bp);
                STAILQ_INSERT_TAIL(&pconn->pipe1, buf, entries);
                goto recv2buf;
            }
        }
    }

    if(pconn->status == CONN_NONE){
        TAILQ_INSERT_TAIL(&(pconn->ln->cq_waiting), pconn, entries);
        pconn->ln->cq_waiting_num ++;
        pconn->status = CONN_WAITING;
        x_log_debug("push into waiting queue!");
        x_upstream_handle(pconn->ln);
    }

    if(pconn->upc_status == CONN_WORKING){
        x_upstream_send_to_peer(pconn);
    }

}

int x_access_send_to_client(xtp_conn_t *pconn)
{
    ev_io_start(pconn->ln->loop, &pconn->e_send);
    x_log_debug("x_access_send_to_client! ");
    return XNET_OK;
}

static void x_send_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, e_send);
    xtp_buf_t	*buf = NULL;
    int len = 0;

bufsend:
    x_log_debug("send from buf!");
    buf = STAILQ_FIRST(&pconn->pipe2);
    if(!buf){
        if(pconn->upc_status == CONN_CLOSE){
            x_event_close(pconn, 1);
        }else{
            ev_io_stop(loop, w);
        }
        return;
    }

    while( buf->pos != buf->last )
    {
        len = x_sendn(pconn->fd, buf->pos, buf->last - buf->pos);
        if(len < 0){
            // error
            x_event_close(pconn, 1);
            return;
        }else if (len == 0){
            // EAGAIN
            return;
        }else{
            buf->pos += len;
        }
    }
    if(buf->pos == buf->end){
        STAILQ_REMOVE(&pconn->pipe2, buf, xtp_buf_s, entries);
        x_buf_free(buf);
        buf = NULL;
        goto bufsend;
    }

    if(pconn->upc_status == CONN_CLOSE){
        x_event_close(pconn, 1);
    }else{
        ev_io_stop(loop, w);
    }

}

/* when timeout, free the connection */
static void x_timeout_cb(EV_P_ ev_timer *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, e_timer);
    xtp_buf_t *buf = NULL;
    if(pconn->status != CONN_CLOSE){
        pconn->status = CONN_TIMEOUT;
    }

    x_event_close(pconn, 1);

    ev_timer_stop(EV_A_ w);

    while((buf = STAILQ_FIRST(&pconn->pipe1))){
        STAILQ_REMOVE(&pconn->pipe1, buf, xtp_buf_s, entries);
        x_buf_free(buf);
    }
    while((buf = STAILQ_FIRST(&pconn->pipe2))){
        STAILQ_REMOVE(&pconn->pipe2, buf, xtp_buf_s, entries);
        x_buf_free(buf);
    }

    if(pconn){
        zfree(pconn);
        pconn = NULL;
    }
}

static void x_accept_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_listen_t *listen = (xtp_conn_listen_t *)x_ev_of_conn_listen(w, e_listen);

    int newfd;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(struct sockaddr);

    while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0){
        if (errno == EAGAIN){
            /* these are transient, so don't log anything. */
            continue; 
        }else{
            x_log_err("accept error.[%s]", strerror(errno));
            break;
        }
    }
    if(listen->c_count >= listen->srvcf->access.max){
        x_log_debug("access count=%d, max=%d refused!", listen->c_count, listen->srvcf->access.max);
        close(newfd);
        return;
    }

    xtp_conn_t *pconn = (xtp_conn_t*)zmalloc(sizeof(xtp_conn_t));
    memset(pconn, 0, sizeof(xtp_conn_t));
    unsigned char *ip=(unsigned char *)(&sin.sin_addr.s_addr);
    sprintf(pconn->ip,"%d.%d.%d.%d",*ip,*(ip+1),*(ip+2),*(ip+3));
    pconn->ip[strlen(pconn->ip)] = '\0';
    pconn->fd = newfd;
    pconn->status = CONN_NONE;
    pconn->upc_status = CONN_NONE;
    pconn->ln = listen;

    STAILQ_INIT(&pconn->pipe1);
    STAILQ_INIT(&pconn->pipe2);

    x_setNonBlock(newfd);
    ev_io_init(&pconn->e_recv,x_recv_cb,newfd,EV_READ);
    ev_io_init(&pconn->e_send,x_send_cb,newfd,EV_WRITE);
    ev_io_start(loop,&pconn->e_recv);

    ev_timer_init(&pconn->e_timer, x_timeout_cb, listen->timeout, listen->timeout);
    ev_timer_start(loop, &pconn->e_timer);

    listen->c_count ++;
    x_log_debug("accept callback : fd :%d, c_count: %d",pconn->fd, listen->c_count);
}
void* x_access_in(void *arg)
{
    if (!arg){
        goto endFunc;
    }

    xtp_server_t *srv = (xtp_server_t*)arg;
    xtp_srvconf_t *srvconf = srv->srvconf;
    //    int i = 0;

    xtp_access_entry_t *item = &srvconf->access;
    if(!item)
        goto endFunc;

    xtp_conn_listen_t *listen = (xtp_conn_listen_t*)zmalloc(sizeof(xtp_conn_listen_t));
    if(!listen){
        x_log_err("calloc xtp_conn_listen_t failed!");
        goto endFunc;
    }

    listen->srvcf = srvconf;
    listen->port = item->port;
    listen->fd = x_socket_bind(listen->port);
    listen->c_count = 0;

    TAILQ_INIT(&listen->cq_waiting);
    listen->cq_waiting_num = 0;
    listen->working_num = 0;

    listen->bp = (xtp_buf_pool_t*)zmalloc(sizeof(xtp_buf_pool_t));
    x_buf_init(listen->bp, 1, NODE_BUFSIZ);

    listen->timeout = item->timeout;

    srv->listen = listen;

    /* begin event loop */
    listen->loop = ev_loop_new(EVFLAG_AUTO);
    ev_io_init(&listen->e_listen, x_accept_cb, listen->fd, EV_READ);
    ev_io_start(listen->loop, &listen->e_listen);

    //        ev_async_init(&listen->e_sig, x_sendsig_cb);
    //        ev_async_start(listen->loop, &listen->e_sig);

    x_log_debug("Listen ,loop start");
    /* start event loop */
    ev_loop(listen->loop, 0);

    ev_loop_destroy(listen->loop);
    x_buf_uninit(listen->bp);
    zfree(listen->bp);
    zfree(listen);

endFunc:    
    return NULL;
}

void x_access_new(xtp_server_t *srv)
{
    xtp_thread_t *thr = srv->thr;
    thr->func = x_access_in;
    x_thread_fork(thr, (void*)srv);
}

