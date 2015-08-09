/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"


/* return -2:closed error ; -1:error ; */
int x_recvn(int nSocket , void *pStr , unsigned int nLen)
{
    int n = nLen;
    char *str = (char*)pStr;

    while(n > 0) {
        int nRet = recv(nSocket , str , n , 0);
        if(nRet < 0) {
            if(errno == EINTR) {
                usleep(1000);
                continue;
            } else if(errno == EAGAIN) {
                break;
            } else
                return -1;
        } else if(nRet == 0) {
            return -2;
        }
        n -= nRet;
        str += nRet;
    }
    return (nLen - n);
}

/* return -2:closed error ; -1:error ; */
int x_sendn(int nSocket , const void *pStr , unsigned int nLen)
{
    int n = nLen , nRet;
    char *str = (char*)pStr;
    while(n > 0)
    {
        nRet = send(nSocket , str , n , 0);
        if(nRet < 0){
            if(errno == EINTR){
                usleep(1000);
                continue;
            }
            if(errno != EAGAIN)
                return -1;
            PINT(errno);
            break;
        }else if (nRet == 0)
            return -2;
        n -= nRet;
        str += nRet;
    }
    return (nLen - n);
}

int x_setNonBlock(int sockfd)
{
    int opts = fcntl(sockfd, F_GETFL);
    if(opts == -1)
        return -1;
    opts = opts | O_NONBLOCK;
    if(fcntl(sockfd, F_SETFL, opts)){
        return -1;
    }
    return 0;
}

int x_socket_bind(int port)
{
    struct sockaddr_in my_addr;
    int listener;
    if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        x_log_err("ACCESS SOCKET CREATE ERROR!");
        return -1;
    }

    /* x_log_debug("ACCESS SOCKET CREATE SUCCESS!");    */

    x_setNonBlock(listener);
    int so_reuseaddr=1;
    setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1)
    {
        x_log_err("ACCESS SOCKET BIND ERROR!");
        exit(1);
    }

    if (listen(listener, 1024) == -1)
    {
        x_log_err("ACCESS SOCKET LISTEN ERROR!");
        exit(1);
    }
    else
    {
        x_log_err("LISTEN SUCCESS,PORT:%d",port);
    }
    return listener;
}
