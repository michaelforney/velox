# velox: Makefile

include config.mk

PREFIX          ?= /usr/local
BINDIR          ?= $(PREFIX)/bin
DATADIR         ?= $(PREFIX)/share
LIBDIR          ?= $(PREFIX)/lib
PKGCONFIGDIR    ?= $(LIBDIR)/pkgconfig

PKG_CONFIG      ?= pkg-config
WAYLAND_SCANNER ?= wayland-scanner

VERSION_MAJOR   := 0
VERSION_MINOR   := 1
VERSION         := $(VERSION_MAJOR).$(VERSION_MINOR)

TARGETS         := velox.pc velox
SUBDIRS         := protocol clients
CLEAN_FILES     := $(TARGETS)

VELOX_PACKAGES  = swc xkbcommon
VELOX_SOURCES   =               \
    config.c                    \
    layout.c                    \
    screen.c                    \
    tag.c                       \
    util.c                      \
    velox.c                     \
    window.c                    \
    protocol/swc-protocol.c     \
    protocol/velox-protocol.c

ifeq ($(if $(V),$(V),0), 0)
    define quiet
        @echo "  $1	$@"
        @$(if $2,$2,$($1))
    endef
else
    quiet = $(if $2,$2,$($1))
endif

VELOX_OBJECTS           = $(VELOX_SOURCES:%.c=%.o)
VELOX_PACKAGE_CFLAGS   ?= $(call pkgconfig,$(VELOX_PACKAGES),cflags,CFLAGS)
VELOX_PACKAGE_LIBS     ?= $(call pkgconfig,$(VELOX_PACKAGES),libs,LIBS)

CLEAN_FILES += $(VELOX_OBJECTS)

FINAL_CFLAGS = $(CFLAGS) -std=c99
FINAL_CPPFLAGS = $(CPPFLAGS) -D_POSIX_C_SOURCE=200809L

# Warning/error flags
FINAL_CFLAGS += -Werror=implicit-function-declaration -Werror=implicit-int \
                -Werror=pointer-sign -Werror=pointer-arith \
                -Wall -Wno-missing-braces

ifeq ($(ENABLE_DEBUG),1)
    FINAL_CPPFLAGS += -DENABLE_DEBUG=1
    FINAL_CFLAGS += -g
else
    FINAL_CPPFLAGS += -DNDEBUG
endif

compile     = $(call quiet,CC) $(FINAL_CPPFLAGS) $(FINAL_CFLAGS) -c -o $@ $< \
              -MMD -MP -MF .deps/$(basename $<).d -MT $(basename $@).o
link        = $(call quiet,CCLD,$(CC)) $(LDFLAGS) -o $@ $^
pkgconfig   = $(sort $(foreach pkg,$(1),$(if $($(pkg)_$(3)),$($(pkg)_$(3)), \
                                           $(shell $(PKG_CONFIG) --$(2) $(pkg)))))

.PHONY: all
all: build

include $(foreach dir,$(SUBDIRS),$(dir)/local.mk)

.PHONY: build
build: $(SUBDIRS:%=build-%) $(TARGETS)

.deps:
	@mkdir $@

%.o: %.c | .deps
	$(compile) $(VELOX_PACKAGE_CFLAGS)

# Explicitly state dependencies on generated files
screen.o tag.o: protocol/velox-server-protocol.h

velox: $(VELOX_OBJECTS)
	$(link) $(VELOX_PACKAGE_LIBS) -lm

velox.pc: velox.pc.in
	$(call quiet,GEN,sed)               \
	    -e "s:@VERSION@:$(VERSION):"    \
	    -e "s:@DATADIR@:$(DATADIR):"    \
	    $< > $@

$(foreach dir,BIN PKGCONFIG,$(DESTDIR)$($(dir)DIR)) $(DESTDIR)$(DATADIR)/velox:
	mkdir -p $@

.PHONY: install
install: $(TARGETS) | $(DESTDIR)$(BINDIR) $(DESTDIR)$(PKGCONFIGDIR)
	install -m 755 velox $(DESTDIR)$(BINDIR)
	install -m 644 velox.pc $(DESTDIR)$(PKGCONFIGDIR)

.PHONY: clean
clean:
	rm -rf $(CLEAN_FILES)

-include .deps/*.d

