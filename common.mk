# velox: common.mk

.PHONY: build-$(dir)
build-$(dir): $($(dir)_TARGETS)

.deps/$(dir): | .deps
	@mkdir "$@"

$(dir)/%: dir := $(dir)

$(dir)/%.o: $(dir)/%.c | .deps/$(dir)
	$(compile) -I. $($(dir)_PACKAGE_CFLAGS)

ifdef $(dir)_PACKAGES
    ifndef $(dir)_PACKAGE_CFLAGS
        $(dir)_PACKAGE_CFLAGS := $(call pkgconfig,$($(dir)_PACKAGES),cflags,CFLAGS)
    endif
    ifndef $(dir)_PACKAGE_LIBS
        $(dir)_PACKAGE_LIBS := $(call pkgconfig,$($(dir)_PACKAGES),libs,LIBS)
    endif
endif

