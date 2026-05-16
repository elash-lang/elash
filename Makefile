CC ?= cc
BUILD ?= release

ifeq ($(OS),Windows_NT)
	PLATFORM := windows
else
	PLATFORM := posix
endif

ifeq ($(wildcard VERSION),)
$(error VERSION file not found)
endif

ifeq ($(PLATFORM),windows)
	VERSION   := $(strip $(shell type VERSION))
	DIST_OS   := windows
	DIST_ARCH := $(shell powershell -NoProfile -Command "$$env:PROCESSOR_ARCHITECTURE.ToLower()")
else
	VERSION   := $(strip $(shell cat VERSION))
	DIST_OS   := $(shell uname -s | tr '[:upper:]' '[:lower:]')
	DIST_ARCH := $(shell uname -m | tr '[:upper:]' '[:lower:]')
endif

SRC_DIR     := src
TEST_DIR    := tests
INCLUDE_DIR := include

OBJ_ROOT_DIR := build/$(BUILD)/obj
DEP_ROOT_DIR := build/$(BUILD)/dep

OUT_DIR     := out/$(BUILD)
LIB_DIR     := $(OUT_DIR)/lib
BIN_DIR     := $(OUT_DIR)/bin

EXE_EXT :=
ifeq ($(PLATFORM),windows)
	EXE_EXT    := .exe
	SHARED_EXT := .dll
	STATIC_EXT := .lib
else ifeq ($(PLATFORM),posix)
	EXE_EXT    :=
	SHARED_EXT := .so
	STATIC_EXT := .a
endif

LIBELASH_NAME    := elash
LIBELASH_STATIC  := $(LIB_DIR)/lib$(LIBELASH_NAME)$(STATIC_EXT)
LIBELASH_SHARED  := $(LIB_DIR)/lib$(LIBELASH_NAME)$(SHARED_EXT)

LIBELC_NAME      := elc
LIBELC_STATIC    := $(LIB_DIR)/lib$(LIBELC_NAME)$(STATIC_EXT)
LIBELC_SHARED    := $(LIB_DIR)/lib$(LIBELC_NAME)$(SHARED_EXT)

DIST_NAME := $(OUT_DIR)/elash-$(VERSION)-$(DIST_OS)-$(DIST_ARCH)-$(BUILD)

ifeq ($(PLATFORM),windows)
	DIST_FILE := $(DIST_NAME).zip
else
	DIST_FILE := $(DIST_NAME).tar.gz
endif

ELC_BIN := $(BIN_DIR)/elc$(EXE_EXT)

CSTD     := -std=c11
WARNINGS := -Wall -Wextra -Werror=implicit-fallthrough
PIC_CFLAGS := -fPIC

COMMON_CFLAGS := $(CSTD) $(WARNINGS) -I$(INCLUDE_DIR)

ifeq ($(BUILD),debug)
	CFLAGS := $(COMMON_CFLAGS) -O0 -g -DEL_DEBUG
	LDFLAGS :=
else ifeq ($(BUILD),debug-san)
	CFLAGS := $(COMMON_CFLAGS) -O0 -g -DEL_DEBUG -fsanitize=address,undefined
	LDFLAGS := -fsanitize=address,undefined
else ifeq ($(BUILD),release)
	CFLAGS := $(COMMON_CFLAGS) -O3 -DNDEBUG
	LDFLAGS :=
else ifeq ($(BUILD),rel-debug)
	CFLAGS := $(COMMON_CFLAGS) -O3 -g -DNDEBUG
	LDFLAGS :=
else ifeq ($(BUILD),rel-debug-san)
	CFLAGS := $(COMMON_CFLAGS) -O3 -g -DNDEBUG -fsanitize=address,undefined
	LDFLAGS := -fsanitize=address,undefined
else
	$(error Unknown BUILD=$(BUILD))
endif

# LLVM Configuration
LLVM_CONFIG ?= llvm-config
HAS_LLVM    := $(shell $(LLVM_CONFIG) --version > /dev/null 2>&1 && echo yes || echo no)

ifeq ($(HAS_LLVM),yes)
	LLVM_CFLAGS  := $(shell $(LLVM_CONFIG) --cflags)
	LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --libs --system-libs)
	ifeq ($(PLATFORM),posix)
		LLVM_LDFLAGS += -lstdc++
	endif
else
	LLVM_CFLAGS  :=
	LLVM_LDFLAGS :=
endif

ifeq ($(PLATFORM),windows)
	CMD_MKDIR_P = powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(subst /,\,$(1))' | Out-Null"
	CMD_RM_RF   = powershell -NoProfile -Command "Remove-Item -Recurse -Force -Path '$(subst /,\,$(1))' | Out-Null"
	CMD_ARCHIVE = powershell -NoProfile -Command "Compress-Archive -Path '$(subst /,\,$(OUT_DIR)/*)' -DestinationPath '$(subst /,\,$(DIST_FILE))' -Force"
	FIXPATH     = $(subst /,\,$(1))
else ifeq ($(PLATFORM),posix)
	CMD_MKDIR_P = mkdir -p "$(1)"
	CMD_RM_RF = rm -rf "$(1)"
	CMD_ARCHIVE = tar -czf $(DIST_FILE) -C $(OUT_DIR) bin lib
	FIXPATH     = $(1)
endif

rwildcard = \
	$(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) \
	$(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

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

# libelc and elc depend on LLVM
$(LIBELC_OBJ_STATIC) $(LIBELC_OBJ_SHARED) $(MAIN_OBJ): CFLAGS += $(LLVM_CFLAGS)
$(LIBELC_SHARED): LDFLAGS += $(LLVM_LDFLAGS)
$(ELC_BIN): LDFLAGS += $(LLVM_LDFLAGS)

DEPS := $(patsubst %.c,$(DEP_ROOT_DIR)/%.d,$(ALL_C_SRCS)) \
        $(patsubst %.c,$(DEP_ROOT_DIR)/shared/%.d,$(LIBELASH_C_SRCS) $(LIBELC_C_SRCS))

.PHONY: all dirs lint clean check-llvm
.PHONY: libelash libelash-shared libelash-static
.PHONY: libelc libelc-shared libelc-static
.PHONY: elc archive

all: dirs elc libelash libelc

archive: all
	@$(call CMD_ARCHIVE)

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

$(LIBELASH_STATIC): $(LIBELASH_OBJ_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))
	ar rcs $@ $^

$(LIBELASH_SHARED): $(LIBELASH_OBJ_SHARED)
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) -shared $^ $(LDFLAGS) -o $@

$(LIBELC_STATIC): $(LIBELC_OBJ_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))
	ar rcs $@ $^

$(LIBELC_SHARED): $(LIBELC_OBJ_SHARED) $(LIBELASH_SHARED)
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) -shared $(LIBELC_OBJ_SHARED) -L$(LIB_DIR) -lelash $(LDFLAGS) -o $@

$(ELC_BIN): $(MAIN_OBJ) $(LIBELC_STATIC) $(LIBELASH_STATIC)
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) $(MAIN_OBJ) $(LIBELC_STATIC) $(LIBELASH_STATIC) $(LDFLAGS) -o $@

$(OBJ_ROOT_DIR)/%.o: %.c
	@$(call CMD_MKDIR_P,$(dir $@))
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR)/$(dir $<))
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/$*.d -c $< -o $@

$(OBJ_ROOT_DIR)/shared/%.o: %.c
	@$(call CMD_MKDIR_P,$(dir $@))
	@$(call CMD_MKDIR_P,$(DEP_ROOT_DIR)/shared/$(dir $<))
	$(CC) $(CFLAGS) $(PIC_CFLAGS) -MMD -MP -MF $(DEP_ROOT_DIR)/shared/$*.d -c $< -o $@

-include $(DEPS)

include tests/build.mk

clean:
	@$(call CMD_RM_RF,build)
	@$(call CMD_RM_RF,out)
