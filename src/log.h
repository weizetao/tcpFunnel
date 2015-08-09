#ifndef LOG_H
#define LOG_H

#include "xtp_config.h"
#include "xtp_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Available logging levels.
 */
enum log_levels {
	LOG_LEVEL_NORMAL = 0,
	LOG_LEVEL_DEBUG,    /**< Debug. */
	LOG_LEVEL_INFO,     /**< Informational. */
	LOG_LEVEL_WARN,     /**< Warning. */
	LOG_LEVEL_ERR,      /**< Error. */
	LOG_LEVEL_NONE,     /**< NONE LOG. */
};

/**
 * Log an Warning message.
 */
void x_log_warn(const char *fmt, ...);

/**
 * Log a error message.
 */
void x_log_err(const char *fmt, ...);

/**
 * Log an information message.
 */
void x_log_info(const char *fmt, ...);

/**
 * Log a debug message.
 */
void x_log_debug(const char *fmt, ...);

void logger_init(int filesize, int is_debug, const char *root);

void change_log_path(int level);

void x_log_free();
void set_log_level(int level);

#ifdef __cplusplus
}
#endif
#endif
