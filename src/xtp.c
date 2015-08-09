/*
 * Copyright (C) weizetao
 */

#include "xtp_config.h"
#include "xtp_core.h"


static void signal_handler(int sig){
    pid_t pid = 0;
    pid = getpid();
    switch(sig){
        case SIGINT:
            x_log_err("SIGINT, quit! pid=%d", pid);
            exit(0);
            //			break;
        case SIGQUIT:
            x_log_err("SIGQUIT, quit! pid=%d", pid);
            exit(0);
            //			break;
        case SIGTERM:
            // x_log_free();
            x_log_err("SIGTERM, quit! pid=%d", pid);
            exit(0);
            break;
        case SIGHUP:
            //	         x_log_free();
            x_log_err("SIGHUP, quit! pid=%d", pid);
            exit(0);
            //			break;
        case SIGALRM:
            // process update
            x_log_debug("xtp: recv SIGALRM signal(pid=%d)", pid);
            //			break;
            exit(0);
        default:
            x_log_err("xtp recv %d signal! pid=%d", pid);
            break;
    }
}

void show_help() {
    const char *help = "\n"
        "-c           the path of config file\n"
        "-d           run as a deamon\n"
        "-L <path>    log path\n"
        "-l <level>   debug log_levels:DEBUG,WARN,INFO,ERR,NONE\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr, "%s", help);
}

int main(int argc, char *argv[])
{
    int xtp_option_daemon = 0;
    int c = 0;
    char *conf_path = NULL;
    char *log_path = NULL;
    enum log_levels log_level = LOG_LEVEL_DEBUG;

    //signal handle
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGQUIT,  signal_handler);
    signal(SIGTERM,  signal_handler);
    signal(SIGALRM,  signal_handler);

    srand(time(NULL));

    //获取参数
    while ((c = getopt(argc, argv, "c:L:l:dh")) != -1) {
        switch (c) {
            case 'c' :
                conf_path = optarg;
                break;
            case 'L' :
                if(x_file_isdir(optarg)){
                    log_path = optarg;
                }else{
                    fprintf(stderr, "-L:\"%s\" is not a directory\n", optarg);
                    exit(-1);
                }
                break;
            case 'l':
                if(strncmp(optarg, "DEBUG", 5) == 0){
                    log_level = LOG_LEVEL_DEBUG;
                }else if(strncmp(optarg, "WARN", 4) == 0){
                    log_level = LOG_LEVEL_WARN;
                }else if(strncmp(optarg, "INFO", 4) == 0){
                    log_level = LOG_LEVEL_INFO;
                }else if(strncmp(optarg, "ERR", 3) == 0){
                    log_level = LOG_LEVEL_ERR;
                }else if(strncmp(optarg, "NONE", 4) == 0){
                    log_level = LOG_LEVEL_NONE;
                }else{
                    fprintf(stderr, "%s\n", "UNDOWN LOG LEVEL!");
                    log_level = LOG_LEVEL_NONE;
                }
                break;
            case 'd' :
                xtp_option_daemon = 1;
                break;
            case 'h' :
            default :
                show_help();
                exit(0);
        }
    }

    if(conf_path == NULL){
        conf_path = DEFAULT_CONF_FILE;
    }
    if(!x_file_exists(conf_path)){
        fprintf(stderr, "-c:\"%s\" file is not exists\n", conf_path);
        exit(-1);
    }

    if (xtp_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(-1);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(0);
        }
    }

    logger_init(100, LOG_LEVEL_DEBUG, log_path);

    xtp_server_t *g_srv = NULL;

    g_srv = x_server_new();
    if(!g_srv) {
        x_log_err("Server init failed!");
        x_log_free();
        return -1;
    }

    g_srv->srvconf = x_server_conf_new(conf_path);
    if(!g_srv->srvconf){
        x_log_err("Server configure init failed!");
        x_server_destroy(g_srv);
        x_log_free();
        return -1;
    }

    set_log_level(log_level);

    x_access_new(g_srv);

    while(1){
        sleep(10);
    }
    x_log_free();
    return 0;
}
