# velox: config.mk

CC                  = clang
CFLAGS              = -pipe

ENABLE_DEBUG        = 1

swc_LIBS = /home/michael/scm/swc/libswc/libswc.so -lwayland-server -lxkbcommon
swc_CFLAGS = -I/home/michael/scm/swc/libswc

