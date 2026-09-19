.PHONY: install uninstall

PREFIX     ?= /usr/local
DESTDIR    ?=
BINDIR     ?= $(PREFIX)/bin
LIBDIR     ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

install: all
ifneq ($(PLATFORM),posix)
	$(error install unsupported on non-posix platforms)
else
	@$(call CMD_MKDIR_P,$(DESTDIR)$(BINDIR))
	@$(call CMD_MKDIR_P,$(DESTDIR)$(LIBDIR))
	@$(call CMD_MKDIR_P,$(DESTDIR)$(INCLUDEDIR))

	$(ECHO) "INSTALL $(DESTDIR)$(BINDIR)/$(notdir $(ELC_BIN))"
	$(Q)install -m755 $(ELC_BIN) $(DESTDIR)$(BINDIR)/

	$(ECHO) "INSTALL $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_STATIC))"
	$(Q)install -m644 $(LIBELASH_STATIC) $(DESTDIR)$(LIBDIR)/

	$(ECHO) "INSTALL $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_SHARED))"
	$(Q)install -m755 $(LIBELASH_SHARED) $(DESTDIR)$(LIBDIR)/

	$(ECHO) "INSTALL $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_STATIC))"
	$(Q)install -m644 $(LIBELC_STATIC) $(DESTDIR)$(LIBDIR)/

	$(ECHO) "INSTALL $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_SHARED))"
	$(Q)install -m755 $(LIBELC_SHARED) $(DESTDIR)$(LIBDIR)/

	$(ECHO) "INSTALL $(DESTDIR)$(INCLUDEDIR)"
	$(Q)cp -R $(INCLUDE_DIR)/. $(DESTDIR)$(INCLUDEDIR)/
endif

uninstall:
ifneq ($(PLATFORM),posix)
	$(error install unsupported on non-posix platforms)
else
	$(ECHO) "RM $(DESTDIR)$(BINDIR)/$(notdir $(ELC_BIN))"
	@$(call CMD_RM_F,$(DESTDIR)$(BINDIR)/$(notdir $(ELC_BIN)))

	$(ECHO) "RM $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_STATIC))"
	@$(call CMD_RM_F,$(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_STATIC)))

	$(ECHO) "RM $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_SHARED))"
	@$(call CMD_RM_F,$(DESTDIR)$(LIBDIR)/$(notdir $(LIBELASH_SHARED)))

	$(ECHO) "RM $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_STATIC))"
	@$(call CMD_RM_F,$(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_STATIC)))

	$(ECHO) "RM $(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_SHARED))"
	@$(call CMD_RM_F,$(DESTDIR)$(LIBDIR)/$(notdir $(LIBELC_SHARED)))

	$(ECHO) "RM $(DESTDIR)$(INCLUDEDIR)/elash"
	@$(call CMD_RM_RF,$(DESTDIR)$(INCLUDEDIR)/elash)
endif
