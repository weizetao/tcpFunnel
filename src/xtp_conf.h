/*
 * Copyright (C) weizetao
 */

#ifndef _XNET_CONF_H
#define _XNET_CONF_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRX_FRONTEND_IN    0
#define PRX_BACKEND_IN     1
#define	PRX_FRONTEND_OUT   2
#define PRX_BACKEND_OUT    3

#define PAN_BACKEND_IN   0
#define PAN_BACKEND_OUT  1

typedef struct xtp_access_entry_s {
    char    *ip;
    int     port;
    int     max;
    int     timeout;
} xtp_access_entry_t;

typedef struct xtp_upstream_conf_s {
    char    *ip;
    int     port;
    int     timeout;
} xtp_upstream_conf_t;

#define WOR_IN  0
#define WOR_OUT 1

struct xtp_srvconf_s{
	char					*pidfile;
	iks						*conf;
    xtp_access_entry_t     access;

    xtp_upstream_conf_t	**upstream;
    int						upstream_num;
    int						upstream_index;
    int						working_num_max;
};

xtp_srvconf_t *x_server_conf_new(const char *conf_file);
iks *x_get_module_conf(xtp_srvconf_t *srvconf, const char *name);
void x_server_conf_destroy(xtp_srvconf_t *srvconf);
int x_get_socket_mode(char *s);
int x_file_exists(const char *fname);
int x_file_isdir(const char *fpath);


#ifdef __cplusplus
}
#endif

#endif

