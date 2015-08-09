/*
 * Copyright (C) weizetao
 */


#include "xtp_config.h"
#include "xtp_core.h"

xtp_server_t *x_server_new()
{
    xtp_server_t *srv = (xtp_server_t *)zmalloc(sizeof(xtp_server_t));
    if(!srv){
        return NULL;
    }



    srv->thr = (xtp_thread_t *)zmalloc(sizeof(xtp_thread_t));

    return srv;
}

void x_server_destroy(xtp_server_t *srv)
{
    if(!srv) return;

    zfree(srv->thr);
    zfree(srv);
    return;
}
