/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"


xtp_srvconf_t *x_server_conf_new(const char *conf_file)
{
    if(!conf_file) return NULL;

    iks *conf = NULL, *sub_child = NULL;
    char *content = NULL;
    xtp_srvconf_t *srvconf = NULL;
    int i = 0;


    if(iks_load(conf_file, &conf) != IKS_OK){
        x_log_err("Load conf failed! ");
        goto err_todo;
    }

    srvconf = (xtp_srvconf_t *)zmalloc(sizeof(xtp_srvconf_t));
    if(!srvconf){
        x_log_err("calloc failed!File:");
        goto err_todo;
    }
    memset(srvconf, 0, sizeof(xtp_srvconf_t));

    srvconf->conf = conf;

    if((content = iks_find_cdata(conf, "pidfile"))){
        srvconf->pidfile = content;
    } else {
        x_log_warn("Can't find pidfile node!");
        srvconf->pidfile = (char  *) XNET_DEFAULE_PIDFILE;
    }


    iks *server = iks_find(conf, "server");
    iks *access = iks_find(server, "access");
    if((content = iks_find_attrib(access, "port")) != NULL){
        srvconf->access.port = atoi(content);
    } else {
        x_log_warn("Can't find access port");
        srvconf->access.port = 9097;
    }
    if((content = iks_find_attrib(access, "max")) != NULL){
        srvconf->access.max = atoi(content);
    } else {
        x_log_warn("Can't find access port");
        srvconf->access.max = CONN_NUM_MAX;
    }
    if((content = iks_find_attrib(access, "timeout")) != NULL){
        srvconf->access.timeout = atoi(content);
    } else {
        x_log_warn("Can't find access timeout");
        srvconf->access.timeout = 60;
    }

    iks *upstream = iks_find(server, "upstream");
    srvconf->upstream_num = 0;
    srvconf->upstream_index = 0;
    for(sub_child = iks_child(upstream); sub_child; sub_child = iks_next(sub_child)){
        if(strchr(iks_name(sub_child), '\n')) continue;
        srvconf->upstream_num ++;
    }
    srvconf->upstream = (xtp_upstream_conf_t **)zmalloc(sizeof(xtp_upstream_conf_t*) * srvconf->upstream_num);
    for(sub_child = iks_child(upstream),i=0; sub_child; sub_child = iks_next(sub_child)){
        if(strchr(iks_name(sub_child), '\n')) continue;
        srvconf->upstream[i] = (xtp_upstream_conf_t *)zmalloc(sizeof(xtp_upstream_conf_t));

        if((content = iks_find_attrib(sub_child, "ip")) != NULL){
            srvconf->upstream[i]->ip = content;
        } else {
            x_log_err("Can't find upstream server ip");
            goto err_todo;
        }
        if((content = iks_find_attrib(sub_child, "port")) != NULL){
            srvconf->upstream[i]->port = atoi(content);
        } else {
            x_log_warn("Can't find access port, default 80");
            srvconf->upstream[i]->port = 80;
        }
        if((content = iks_find_attrib(sub_child, "timeout")) != NULL){
            srvconf->upstream[i]->timeout = atoi(content);
        } else {
            x_log_warn("Can't find upstream timeout");
            srvconf->upstream[i]->timeout = 60;
        }
        ++i;
    }
    if((content = iks_find_attrib(upstream, "max")) != NULL){
        srvconf->working_num_max = atoi(content);
    } else {
        srvconf->working_num_max = CONN_NUM_MAX;
    }

    return srvconf;

err_todo:
    if(conf)
        iks_delete(conf);
    if(srvconf)
        zfree(srvconf);
    return NULL;
}

void x_server_conf_destroy(xtp_srvconf_t *srvconf)
{
    if(!srvconf) return;
    int i;

    iks_delete(srvconf->conf);

    for(i=0; i < srvconf->upstream_num; i++){
        if(srvconf->upstream[i])
            zfree(srvconf->upstream[i]);
    }
    zfree(srvconf->upstream);
    zfree(srvconf);

    return;
}
int x_file_exists(const char *fname)
{
    int rc = 0;
    if (!fname)
        return 0;
    rc = access( fname, F_OK);
    if ( rc != -1 ){
        return 1;
    }else{
        return 0;
    }
}

int x_file_isdir(const char *fpath)
{
    if ( !fpath )
        return 0;

    struct stat s;
    int err = stat(fpath, &s);
    if(-1 == err) {
        return 0;
    } else {
        if(S_ISDIR(s.st_mode)) {
            /* it's a dir */
            return 1;
        } else {
            /* exists but is no dir */
            return 0;
        }
    }
}
