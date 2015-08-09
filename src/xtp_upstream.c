/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"

static void x_upstream_connected_cb(EV_P_ ev_io *w, int revents);
static void x_upstream_send_cb(EV_P_ ev_io *w, int revents);
static void x_upstream_recv_cb(EV_P_ ev_io *w, int revents);
static void x_upstream_timeout_cb(EV_P_ ev_timer *w, int revents);
void x_upstream_final(xtp_conn_t *pconn);
int x_upstream_connection_new(xtp_conn_t *pconn, xtp_upstream_conf_t *upcf);
xtp_upstream_conf_t* x_upstream_choose_peer(xtp_srvconf_t *srvcf);
int x_upstream_init(xtp_conn_t *pconn);
void x_upstream_send_to_peer(xtp_conn_t *pconn);

int x_upstream_handle(xtp_conn_listen_t *ln)
{
    xtp_srvconf_t *srvcf = NULL;
    xtp_conn_t *pconn = NULL;
    int rc = 0;

    x_log_debug("x_upstream_handle!");

    if(TAILQ_EMPTY(&ln->cq_waiting)){
        return XNET_OK;
    }

    srvcf = ln->srvcf;
    x_log_debug("upstream handle, working_num=%d/%d", ln->working_num, srvcf->working_num_max);
    if(ln->working_num >= srvcf->working_num_max){
        return XNET_BUSY;
    }

    pconn = TAILQ_FIRST(&ln->cq_waiting);

    rc = x_upstream_init(pconn);
    if(rc == XNET_OK){
        TAILQ_REMOVE(&ln->cq_waiting, pconn, entries);
        ln->cq_waiting_num --;
        pconn->status = CONN_WORKING;
        ln->working_num ++;
    }else{
        x_upstream_final(pconn);
    }

    return rc;
}
void x_upstream_send_to_peer(xtp_conn_t *pconn)
{
    ev_io_start(pconn->ln->loop,&pconn->upc_e_send);

    return;
}

int x_upstream_init(xtp_conn_t *pconn)
{

    xtp_upstream_conf_t *upcf = NULL;
    int rc = 0;
    x_log_debug("x_upstream_init!");
    upcf = x_upstream_choose_peer(pconn->ln->srvcf);

    rc = x_upstream_connection_new(pconn, upcf);

    pconn->upcf = upcf;

    return rc;
}

void x_upstream_final(xtp_conn_t *pconn)
{
    x_log_debug("x_upstream_final! ");
    if(pconn->upc_status == CONN_NONE){
        x_log_debug("x_upstream_close121!");
        return;
    }
    if(pconn->upc_status != CONN_CLOSE){
        x_log_debug("x_upstream_close!");
        ev_io_stop(pconn->ln->loop, &pconn->upc_e_recv);
        ev_io_stop(pconn->ln->loop, &pconn->upc_e_send);
        ev_io_stop(pconn->ln->loop, &pconn->upc_e_connected);
        ev_timer_stop(pconn->ln->loop, &pconn->upc_e_timer);
        close(pconn->upc_fd);
        pconn->upc_status = CONN_CLOSE;
        if(pconn->status != CONN_CLOSE){
            x_access_send_to_client(pconn);
        }

        pconn->ln->working_num --;
        x_upstream_handle(pconn->ln);
    }
}

int x_upstream_connection_new(xtp_conn_t *pconn, xtp_upstream_conf_t *upcf)
{
    int sockfd = 0;
    int rc = 0;
    struct sockaddr_in peer_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
        return XNET_ERR;

    if(x_setNonBlock(sockfd) == -1){
        return XNET_ERR;
    }

    pconn->upc_fd = sockfd;
    pconn->upc_status = CONN_CONNECT;

    ev_io_init(&pconn->upc_e_recv,x_upstream_recv_cb,sockfd,EV_READ);
    ev_io_init(&pconn->upc_e_send,x_upstream_send_cb,sockfd,EV_WRITE);
    ev_io_init(&pconn->upc_e_connected,x_upstream_connected_cb,sockfd,EV_WRITE);
    ev_io_start(pconn->ln->loop,&pconn->upc_e_recv);
    ev_io_start(pconn->ln->loop,&pconn->upc_e_connected);

    ev_timer_init(&pconn->upc_e_timer, x_upstream_timeout_cb, CONN_CONNECT_TIMEOUT, CONN_CONNECT_TIMEOUT);
    ev_timer_start(pconn->ln->loop, &pconn->upc_e_timer);

    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(upcf->ip);
    peer_addr.sin_port = htons(upcf->port);

    rc = connect(sockfd, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    if(rc == -1){
        if(errno != EINPROGRESS){
            return XNET_ERR;
        }
    }

    return XNET_OK;
}

/* ���锟界��锟界��锟芥�达拷 */
xtp_upstream_conf_t* x_upstream_choose_peer(xtp_srvconf_t *srvcf)
{
    xtp_upstream_conf_t *upcf = NULL;

    if(!srvcf)
        return NULL;

    srvcf->upstream_index = srvcf->upstream_index % srvcf->upstream_num;
    upcf = srvcf->upstream[srvcf->upstream_index];
    srvcf->upstream_index ++;

    x_log_debug("x_upstream_choose_peer! peer:%s:%d", upcf->ip, upcf->port);

    return upcf;
}

static void x_upstream_connected_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, upc_e_connected);
    int valopt;
    socklen_t lon;

    lon = sizeof(int);
    if (getsockopt(pconn->upc_fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
        x_log_err("Error in getsockopt() %d - %s", errno, strerror(errno));
        x_upstream_final(pconn);
        return;
    }
    if (valopt){
        x_log_err("upstream connect failed!getsockopt %d", errno, valopt);
        x_upstream_final(pconn);
        return;
    }

    x_log_debug("upstream connected!");

    ev_io_stop(pconn->ln->loop, &pconn->upc_e_connected);
    ev_io_start(pconn->ln->loop,&pconn->upc_e_send);
    pconn->upc_e_timer.repeat = pconn->upcf->timeout;
    ev_timer_again(pconn->ln->loop, &pconn->upc_e_timer);

    pconn->upc_status = CONN_WORKING;
}

static void x_upstream_send_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, upc_e_send);
    xtp_buf_t	*buf = NULL;
    int len = 0;

bufsend:
    buf = STAILQ_FIRST(&pconn->pipe1);
    if(!buf){
        ev_io_stop(loop, w);
        return;
    }

    while( buf->pos != buf->last )
    {
        len = x_sendn(pconn->upc_fd, buf->pos, buf->last - buf->pos);
        x_log_debug("send to upstream!len=%d", len);
        if(len < 0){
            // error
            x_upstream_final(pconn);
            return;
        }else if (len == 0){
            // EAGAIN
            break;
        }else{
            buf->pos += len;
        }
    }
    if(buf->pos == buf->end){
        STAILQ_REMOVE(&pconn->pipe1, buf, xtp_buf_s, entries);
        x_buf_free(buf);
        buf = NULL;
        goto bufsend;
    }

    ev_io_stop(loop, w);
    return;
}

static void x_upstream_recv_cb(EV_P_ ev_io *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, upc_e_recv);
    xtp_buf_t *buf = NULL;
    int len = 0;
    x_log_debug("recv from upstream!");
    buf = STAILQ_LAST(&pconn->pipe2, xtp_buf_s, entries);
    if(!buf){
        buf = x_buf_alloc(pconn->ln->bp);
        STAILQ_INSERT_TAIL(&pconn->pipe2, buf, entries);
    }

recv2buf:
    while(buf->last != buf->end){
        len = x_recvn(pconn->upc_fd, buf->last, buf->end - buf->last);
        if(len < 0){
            x_upstream_final(pconn);
            return;
        }
        else if (len == 0){
            break;
        }else{
            buf->last += len;
            if(buf->end == buf->last){
                buf = x_buf_alloc(pconn->ln->bp);
                STAILQ_INSERT_TAIL(&pconn->pipe2, buf, entries);
                goto recv2buf;
            }
        }
    }

    //	if(pconn->upc_status == CONN_CONNECT){
    //		pconn->upc_status = CONN_WORKING;
    //	}

    x_access_send_to_client(pconn);

    return;
}

static void x_upstream_timeout_cb(EV_P_ ev_timer *w, int revents)
{
    xtp_conn_t *pconn = (xtp_conn_t*)x_ev_of_conn(w, upc_e_timer);
    x_log_debug("upstream timeout!");
    if(pconn->upc_status != CONN_CLOSE){
        pconn->upc_status = CONN_TIMEOUT;
    }

    x_upstream_final(pconn);

    ev_timer_stop(EV_A_ w);

}
