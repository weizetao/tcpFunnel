/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_CONFIG_H
#define _XNET_CONFIG_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* pread(), pwrite(), gethostname() */
#endif

#define _FILE_OFFSET_BITS  64

#define  XNET_OK          0
#define  XNET_ERR      -1
#define  XNET_AGAIN      -2
#define  XNET_BUSY       -3
#define  XNET_DONE       -4
#define  XNET_DECLINED   -5
#define  XNET_ABORT      -6

#include "ev.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>             /* offsetof() */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
// #include <glob.h>
// #include <sys/vfs.h>            /* statfs() */

#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>        /* TCP_NODELAY, TCP_CORK */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#include <time.h>               /* tzset() */
// #include <malloc.h>             /* memalign() */
#include <limits.h>             /* IOV_MAX */
#include <sys/ioctl.h>
// #include <sys/sysctl.h>
// #include <crypt.h>
#include <sys/utsname.h>        /* uname() */
#include <dlfcn.h>

#include <pthread.h>

//#include <sys/queue.h>



#define XNET_MAX_ACCESS_NUM  32
#define XNET_DEFAULE_PIDFILE "/tmp/xtp.pid"
#define XNET_MAX_MODULES       32
#define DEFAULT_CONF_FILE     "etc/conf.xml"
#define XNET_PATH_LEN  1024
#define XNET_LOGNAME_LEN 512

typedef intptr_t        ngx_int_t;
typedef uintptr_t       ngx_uint_t;
typedef intptr_t        ngx_flag_t;
typedef int             ngx_fd_t;

#ifndef NGX_ALIGNMENT
#define NGX_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif

#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


#endif

