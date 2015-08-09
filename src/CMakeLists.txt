SET(SRC_LIST zmalloc.c log.c xtp_conf.c xtp_server.c 
    xtp_bufpool.c xtp_nio.c xtp_access.c xtp_upstream.c
    xtp_thread.c  xtp.c)

ADD_EXECUTABLE(xtp ${SRC_LIST})
TARGET_LINK_LIBRARIES(xtp libiksemel.a ev pthread dl)
