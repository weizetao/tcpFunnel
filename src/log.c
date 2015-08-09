/* For vasprintf(3) on Linux. */
//#define _GNU_SOURCE

#include "xtp_config.h"
#include "xtp_core.h"

char g_path_buf[XNET_PATH_LEN];
char g_err_link_path[XNET_PATH_LEN];
char g_nor_link_path[XNET_PATH_LEN];

struct log_handler{
	int type;

	char filename[FILENAME_MAX];
	FILE *fp;
	int count;
	int cursize;
	pthread_mutex_t mutex_msg;
};

struct logger{
	long long maxsize;
	int max_level;
	int is_debug;
	int is_inited;
	struct log_handler *handler[5];
};

int g_log_level = LOG_LEVEL_NORMAL;

static struct logger logger;
/*
static struct logger logger = {
		.maxsize = 100 * 1024 * 1024,
		.max_level = 5,
		.is_debug = 0,
		.is_inited = 0,
		.handler ={[0 ... 4] = NULL}
};*/

void logger_init(int filesize, int is_debug, const char *root)
{
	memset(g_path_buf, '\0', sizeof(g_path_buf));
	memset(g_err_link_path, '\0', sizeof(g_err_link_path));
	memset(g_nor_link_path, '\0', sizeof(g_nor_link_path));

    if (root == NULL){
	    getcwd(g_path_buf, sizeof(g_path_buf) -1 ); 
    }else{
        strncpy(g_path_buf, root, sizeof(g_path_buf)); 
    }
	
	DIR *dir = NULL;

	time_t t;
	struct tm *ptm = NULL;
	struct tm tmp;
	char tmp_name_buf[XNET_LOGNAME_LEN];
	snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/log", g_path_buf);

	if ((dir = opendir(tmp_name_buf)) == NULL) {
		if (mkdir(tmp_name_buf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
			return;
		}
	}
	else {
		closedir(dir);
	}

    snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/error", g_path_buf);
    if ((dir = opendir(tmp_name_buf)) == NULL) {
        if (mkdir(tmp_name_buf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
            return;
        }
    }
    else {
        closedir(dir);
    }

	//logger.is_inited = 0;
	logger.is_inited = 1;
	logger.max_level = 5;
	logger.is_debug = is_debug;
	logger.maxsize = filesize * 1024 * 1024;
	//logger.maxsize = filesize;
	logger.handler[0] = (struct log_handler *)calloc(1, sizeof(struct log_handler));
	logger.handler[0]->type = 0;
	pthread_mutex_init(&logger.handler[0]->mutex_msg, NULL);

	logger.handler[1] = logger.handler[0];
	logger.handler[2] = logger.handler[0];
	logger.handler[3] = logger.handler[0];

	logger.handler[4] = (struct log_handler *)calloc(1, sizeof(struct log_handler));
	logger.handler[4]->type = 4;
	pthread_mutex_init(&logger.handler[4]->mutex_msg, NULL);


	if ((int)time(&t) != -1 && (ptm = localtime_r(&t, &tmp)) != NULL) {
		snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/error/%d%02d%02d.log", g_path_buf,ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
		if (strcmp(logger.handler[4]->filename, tmp_name_buf) != 0) {
			strncpy(logger.handler[4]->filename, tmp_name_buf, sizeof(logger.handler[4]->filename));
			if (logger.handler[4]->fp != NULL) {
				fclose(logger.handler[4]->fp);
				logger.handler[4]->fp = NULL;
			}
		}
		snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/log/%d_%02d%02d%02d%02d.log", g_path_buf,logger.handler[0]->type,ptm->tm_year + 1900 - 2000, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour);
		if (strcmp(logger.handler[0]->filename, tmp_name_buf) != 0) {
			strncpy(logger.handler[0]->filename, tmp_name_buf, sizeof(logger.handler[0]->filename));
			if (logger.handler[0]->fp != NULL) {
				fclose(logger.handler[0]->fp);
				logger.handler[0]->fp = NULL;
			}
		}

	}

}
void set_log_level(int level)
{
	logger.is_debug = level;
}
void change_log_path(int level)
{
	if(level <= 0 || level == 4 ||  level >=5){
		return;
	}
	time_t t;
	struct tm *ptm = NULL;
	struct tm tmp;
	char tmp_name_buf[XNET_LOGNAME_LEN];


	logger.handler[level] = (struct log_handler *)calloc(1, sizeof(struct log_handler));
	logger.handler[level]->type = level;
	pthread_mutex_init(&logger.handler[level]->mutex_msg, NULL);

	if ((int)time(&t) != -1 && (ptm = localtime_r(&t, &tmp)) != NULL) {
		snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/log/%d_%02d%02d%02d%02d.log", g_path_buf,logger.handler[level]->type,ptm->tm_year + 1900 - 2000, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour);
		if (strcmp(logger.handler[level]->filename, tmp_name_buf) != 0) {
			strncpy(logger.handler[level]->filename, tmp_name_buf, sizeof(logger.handler[level]->filename));
			if (logger.handler[level]->fp != NULL) {
				fclose(logger.handler[level]->fp);
				logger.handler[level]->fp = NULL;
			}
		}

	}

}

static int check_file_size(char *filename)
{
	struct stat f_stat;

	if(stat(filename,&f_stat) == -1){
		return   0;
	}

	return   f_stat.st_size > logger.maxsize;
}

static void fix_log_filename(struct log_handler *hand, int msg_size)
{
	char tmp_name_buf[XNET_LOGNAME_LEN];
	time_t t;
	struct tm *ptm = NULL;
	struct tm tmp;


	if ((int)time(&t) != -1 && (ptm = localtime_r(&t, &tmp)) != NULL) {
		if(hand->type == LOG_LEVEL_ERR){
			snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/error/%d%02d%02d.log", g_path_buf,ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
			snprintf(g_err_link_path, sizeof(g_err_link_path), "%s/error/error.log", g_path_buf);
		} else {
			snprintf(tmp_name_buf, sizeof(tmp_name_buf), "%s/log/%d_%02d%02d%02d%02d.log", g_path_buf,hand->type,ptm->tm_year + 1900 - 2000, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour);
			snprintf(g_nor_link_path, sizeof(g_nor_link_path), "%s/log/Disp.log", g_path_buf);
		}
		if (strncmp(hand->filename, tmp_name_buf, strlen(tmp_name_buf)) != 0) {
			strncpy(hand->filename, tmp_name_buf, sizeof(hand->filename));
			if (hand->fp != NULL) {
				fclose(hand->fp);
				hand->fp = NULL;
			}
		} else {
			if((logger.maxsize > 0) && ((hand->cursize + msg_size) >= logger.maxsize)){
				if(hand->fp != NULL) {
					fclose(hand->fp);
					hand->fp = NULL;
				}
				do{	
					snprintf(tmp_name_buf, sizeof(tmp_name_buf),"%s/log/%d_%02d%02d%02d%02d.log_%d",g_path_buf,hand->type,ptm->tm_year + 1900 - 2000, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,  ++hand->count);
				} while(check_file_size(tmp_name_buf));
				strncpy(hand->filename, tmp_name_buf, sizeof(hand->filename));
				hand->cursize = 0;
			}
		}
	}
}
static void write_log(struct log_handler *hand, char *date, char *msg){
	if(!date || !msg){
		return;
	}

	if(!hand->fp){
		if ((hand->fp = fopen(hand->filename, "a")) == NULL) {
			return;
		}
		if(strstr(hand->filename, "error")){
			unlink(g_err_link_path);
			link(hand->filename, g_err_link_path);
		} else {
			unlink(g_nor_link_path);
			link(hand->filename, g_nor_link_path);
		}
	}
	hand->cursize += fprintf(hand->fp, "%s: %s\n", date, msg);
	fflush(hand->fp);
}

static void logger_log(int log_level, const char *fmt, va_list ap)
{
	struct log_handler *handler = logger.handler[log_level];
	if(!handler){
		return;
	}

	if(log_level != LOG_LEVEL_ERR){

		if(g_log_level > log_level){
			return;
		}
	}

	char date[256]; /* Plenty of space for a date. */
	struct timeval tv;
	struct tm tm;
	char *msg;

	/* Build the time. */
	gettimeofday(&tv, NULL);
	localtime_r((time_t *)&tv.tv_sec, &tm);
	switch(log_level){
		case LOG_LEVEL_INFO:
			snprintf(date, sizeof(date),
					"%d-%02d-%02d %02d:%02d:%02d PID:%d [INFO]",
					1900 + tm.tm_year,
					tm.tm_mon + 1,
					tm.tm_mday,
					tm.tm_hour,
					tm.tm_min,
					tm.tm_sec,
					getpid());
			break;
		case LOG_LEVEL_DEBUG:
			snprintf(date, sizeof(date),
					"%d-%02d-%02d %02d:%02d:%02d PID:%d [DEBUG]",
					1900 + tm.tm_year,
					tm.tm_mon + 1,
					tm.tm_mday,
					tm.tm_hour,
					tm.tm_min,
					tm.tm_sec,
					getpid());
			break;
		case LOG_LEVEL_WARN:
			snprintf(date, sizeof(date),
					"%d-%02d-%02d %02d:%02d:%02d PID:%d [WARN]",
					1900 + tm.tm_year,
					tm.tm_mon + 1,
					tm.tm_mday,
					tm.tm_hour,
					tm.tm_min,
					tm.tm_sec,
					getpid());
			break;
		case LOG_LEVEL_ERR:
			snprintf(date, sizeof(date),
					"%d-%02d-%02d %02d:%02d:%02d PID:%d [ERROR]",
					1900 + tm.tm_year,
					tm.tm_mon + 1,
					tm.tm_mday,
					tm.tm_hour,
					tm.tm_min,
					tm.tm_sec,
					getpid());
			break;
	}
	vasprintf(&msg, fmt, ap);

	if(!msg){
		return;
	}
	pthread_mutex_lock(&handler->mutex_msg);
	fix_log_filename(handler, 256+strlen(msg));

	write_log(handler, date, msg);
	if(logger.is_debug == LOG_LEVEL_DEBUG){
		fprintf(stderr, "%s: %s\n",date, msg);
	}
	pthread_mutex_unlock(&handler->mutex_msg);
	if(msg){
		free(msg);
	}
}

void x_log_free()
{
	int i;
	for(i = 4; i >= 0; i--){
		if(logger.handler[i] && logger.handler[i]->type == i){
			if(logger.handler[i]->fp){
				fclose(logger.handler[i]->fp);
				logger.handler[i]->fp = NULL;
			}
			free(logger.handler[i]);
		}
	}
}

void x_log_info(const char *fmt, ...)
{
	if(!logger.is_inited){
		return;
	}
    if(logger.is_debug > LOG_LEVEL_INFO){
        return;
    }
	va_list ap;
	va_start(ap, fmt);
	logger_log(LOG_LEVEL_INFO, fmt, ap);
	va_end(ap);
}

void x_log_warn(const char *fmt, ...)
{
	if(!logger.is_inited){
		return;
	}

    if(logger.is_debug > LOG_LEVEL_WARN){
        return;
    }
	va_list ap;
	va_start(ap, fmt);
	logger_log(LOG_LEVEL_WARN, fmt, ap);
	va_end(ap);
}
void x_log_err(const char *fmt, ...)
{
	if(!logger.is_inited){
		return;
	}

    if(logger.is_debug > LOG_LEVEL_NONE){
        return;
    }

	va_list ap;
	va_start(ap, fmt);
	logger_log(LOG_LEVEL_ERR, fmt, ap);
	va_end(ap);
}
void x_log_debug(const char *fmt, ...)
{
	if(!logger.is_inited){
		return;
	}
    if(logger.is_debug > LOG_LEVEL_DEBUG){
        return;
    }

	if(logger.is_debug){
		va_list ap;
		va_start(ap, fmt);
		logger_log(LOG_LEVEL_DEBUG, fmt, ap);
		va_end(ap);
	}
}

