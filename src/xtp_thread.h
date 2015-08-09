/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_THREAD_H
#define _XNET_THREAD_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xtp_thread_s {
    pthread_t pid;
    void *arg;
    void* (*func)(void*);
    void* (*start)(void*);
    void* (*stop)(void*);
    void* (*resume)(void*);
    void* (*close)(void*);
};

int x_thread_fork(xtp_thread_t *t, void *arg);
int x_thread_kill(xtp_thread_t *t, void *arg);

#ifdef __cplusplus
}
#endif

#endif
