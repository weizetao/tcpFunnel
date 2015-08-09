/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"

void* x_thread_func(void *arg)
{
    xtp_thread_t *t = (xtp_thread_t*)arg;
    //    pthread_t pid = pthread_self();

    t->func(t->arg);

    return NULL;
}

int x_thread_fork(xtp_thread_t *t, void *arg)
{

    int iRet = 0;
    /* pthread_t pid;    */
    t->arg = arg;
    iRet = pthread_create(&t->pid, NULL, x_thread_func, (void*)t);
    if(iRet == 0){
        iRet = pthread_detach(t->pid);
    }

    return iRet;
}
