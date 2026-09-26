.PHONY: setup-githooks regen-version

VERSION_TEMPLATE := $(SCRIPTS_DIR)/version.h.in
VERSION_OUTPUT   := $(INCLUDE_DIR)/elash/version.h
VERSION_SCRIPT   := $(SCRIPTS_DIR)/regen-version.py

regen-version:
	$(ECHO) "PY $(VERSION_OUTPUT)"
	$(Q)$(PY) $(VERSION_SCRIPT) $(VERSION_FILE) -i $(VERSION_TEMPLATE) -o $(VERSION_OUTPUT)

setup-githooks:
	git config --local core.hooksPath scripts/githooks
ifeq ($(PLATFORM),posix)
	chmod +x scripts/githooks/pre-commit
endif
