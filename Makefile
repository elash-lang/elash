include make/directories.mk
include make/config.mk

LIBELASH_C_SRCS := $(call rwildcard,$(SRC_DIR)/elash,*.c)
LIBELC_C_SRCS := $(filter-out \
  $(SRC_DIR)/elc/main.c, \
  $(call rwildcard,$(SRC_DIR)/elc,*.c))

MAIN_C_SRC      := $(SRC_DIR)/elc/main.c

ALL_C_SRCS := $(LIBELASH_C_SRCS) $(LIBELC_C_SRCS) $(MAIN_C_SRC)

LIBELASH_OBJ_STATIC := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(LIBELASH_C_SRCS))
LIBELASH_OBJ_SHARED := $(patsubst %.c,$(OBJ_ROOT_DIR)/shared/%.o,$(LIBELASH_C_SRCS))

LIBELC_OBJ_STATIC   := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(LIBELC_C_SRCS))
LIBELC_OBJ_SHARED   := $(patsubst %.c,$(OBJ_ROOT_DIR)/shared/%.o,$(LIBELC_C_SRCS))

MAIN_OBJ := $(patsubst %.c,$(OBJ_ROOT_DIR)/%.o,$(MAIN_C_SRC))

# libelc and elc depend on LLVM and libbacktrace
$(LIBELC_OBJ_STATIC) $(LIBELC_OBJ_SHARED) $(MAIN_OBJ): CFLAGS += $(LLVM_CFLAGS) $(BACKTRACE_CFLAGS)
$(LIBELC_SHARED):                                      LDFLAGS += $(LLVM_LDFLAGS) $(BACKTRACE_LDFLAGS)
$(ELC_BIN):                                            LDFLAGS += $(LLVM_LDFLAGS) $(BACKTRACE_LDFLAGS)

DEPS := $(patsubst %.c,$(DEP_ROOT_DIR)/%.d,$(ALL_C_SRCS)) \
        $(patsubst %.c,$(DEP_ROOT_DIR)/shared/%.d,$(LIBELASH_C_SRCS) $(LIBELC_C_SRCS))

.PHONY: all dirs lint clean check-llvm inspect
.PHONY: libelash libelash-shared libelash-static
.PHONY: libelc libelc-shared libelc-static
.PHONY: elc archive

all: dirs compile-commands elc libelash libelc

archive: $(DIST_FILE)
$(DIST_FILE): all
ifeq ($(filter %.zip,$(DIST_FILE)),$(DIST_FILE))
	$(ECHO) "ZIP $@"
else
	$(ECHO) "TAR $@"
endif
	$(Q)$(call CMD_ARCHIVE)

libelash-static: $(LIBELASH_STATIC)
libelash-shared: $(LIBELASH_SHARED)
libelash: libelash-static libelash-shared

libelc-static: check-llvm $(LIBELC_STATIC)
libelc-shared: check-llvm $(LIBELC_SHARED)
libelc: libelc-static libelc-shared

elc: check-llvm $(ELC_BIN)

check-llvm:
ifneq ($(HAS_LLVM),yes)
	$(error LLVM not found. Please install LLVM and ensure $(LLVM_CONFIG) is in your PATH)
endif

dirs:
	@$(call CMD_MKDIR_P,$(LIB_DIR))
	@$(call CMD_MKDIR_P,$(BIN_DIR))
	@$(call CMD_MKDIR_P,$(OBJ_ROOT_DIR))
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR))
	@$(call CMD_MKDIR_P,$(OBJ_ROOT_DIR)/shared)
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR)/shared)

lint:
	clang-tidy $(ALL_C_SRCS) -- $(CFLAGS)
inspect:
	$(MAKE) clean
	$(MAKE) libelash libelc >/dev/null

$(LIBELASH_STATIC): $(LIBELASH_OBJ_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))

	$(ECHO) "AR $@"
	$(Q)$(AR) rcs $@ $^

$(LIBELASH_SHARED): $(LIBELASH_OBJ_SHARED)
	@$(call CMD_MKDIR_P,$(dir $@))

	$(ECHO) "CC $@"
	$(Q)$(CC) -shared $^ $(LDFLAGS) -o $@

$(LIBELC_STATIC): $(LIBELC_OBJ_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))

	$(ECHO) "AR $@"
	$(Q)$(AR) rcs $@ $^

$(LIBELC_SHARED): $(LIBELC_OBJ_SHARED) $(LIBELASH_SHARED)
	@$(call CMD_MKDIR_P,$(dir $@))

	$(ECHO) "LD $@"
	$(Q)$(CC) -shared $(LIBELC_OBJ_SHARED) -L$(LIB_DIR) -lelash $(LDFLAGS) -o $@

$(ELC_BIN): $(MAIN_OBJ) $(LIBELC_STATIC) $(LIBELASH_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))

	$(ECHO) "LD $@"
	$(Q)$(CC) $(MAIN_OBJ) $(LIBELC_STATIC) $(LIBELASH_STATIC) $(LDFLAGS) -o $@

$(OBJ_ROOT_DIR)/%.o: %.c
	@$(call CMD_MKDIR_P,$(dir $@))
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR)/$(dir $<))

	$(ECHO) "CC $@"
	$(Q)$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/$*.d -c $< -o $@

$(OBJ_ROOT_DIR)/shared/%.o: %.c
	@$(call CMD_MKDIR_P,$(dir $@))
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR)/shared/$(dir $<))

	$(ECHO) "CC $@"
	$(Q)$(CC) $(CFLAGS) $(PIC_CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/shared/$*.d -c $< -o $@

-include $(DEPS)

include tests/build.mk
include docs/doxygen.mk

include make/compcmds.mk
include make/utilities.mk
include make/install.mk

clean:
	@$(call CMD_RM_RF,build)
	@$(call CMD_RM_RF,out)
