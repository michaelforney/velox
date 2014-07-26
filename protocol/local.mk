# velox: protocol/local.mk

dir := protocol

PROTOCOL_EXTENSIONS =                                       \
    $(dir)/velox.xml                                        \
    $(call pkgconfig,swc,variable=datadir,DATADIR)/swc.xml

$(dir)_PACKAGES = wayland-server

protocol = $(foreach extension,$(1),protocol/$(extension)-protocol.o)
client_protocol = $(foreach extension,$(1),protocol/$(extension)-client-protocol.h)

define protocol_rules

$(dir)/$$(basename $$(notdir $(1)))-protocol.c: $(1)
	$$(call quiet,GEN,$(WAYLAND_SCANNER)) code < $$< > $$@
$(dir)/$$(basename $$(notdir $(1)))-client-protocol.h: $(1)
	$$(call quiet,GEN,$(WAYLAND_SCANNER)) client-header < $$< > $$@
$(dir)/$$(basename $$(notdir $(1)))-server-protocol.h: $(1)
	$$(call quiet,GEN,$(WAYLAND_SCANNER)) server-header < $$< > $$@

CLEAN_FILES += $(foreach type,protocol.c client-protocol.h server-protocol.h, \
                 $(dir)/$$(basename $$(notdir $(1)))-$(type))

endef

$(eval $(foreach extension,$(PROTOCOL_EXTENSIONS),$(call protocol_rules,$(extension))))

install-$(dir): | $(DESTDIR)$(DATADIR)/velox
	install -m0644 protocol/velox.xml "$(DESTDIR)$(DATADIR)/velox"

include common.mk

