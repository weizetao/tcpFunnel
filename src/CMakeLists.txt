SET(SRC_LIST zmalloc.c log.c xtp_conf.c xtp_server.c 
    xtp_bufpool.c xtp_nio.c xtp_access.c xtp_upstream.c
    xtp_thread.c  xtp.c)

INCLUDE_DIRECTORIES(/usr/local/libev/include)
INCLUDE_DIRECTORIES(/usr/local/iksemel/include)

LINK_DIRECTORIES(/usr/local/libev/lib)
LINK_DIRECTORIES(/usr/local/iksemel/lib)

ADD_EXECUTABLE(xtproxy ${SRC_LIST})
TARGET_LINK_LIBRARIES(xtproxy libiksemel.a ev pthread dl)
