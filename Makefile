CC ?= cc
BUILD ?= release

ifeq ($(OS),Windows_NT)
	PLATFORM := windows
else
	PLATFORM := posix
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

ifeq ($(PLATFORM),windows)
	CMD_MKDIR_P = powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(subst /,\,$(1))' | Out-Null"
	CMD_RM_RF   = powershell -NoProfile -Command "Remove-Item -Recurse -Force -Path '$(subst /,\,$(1))' | Out-Null"
else ifeq ($(PLATFORM),posix)
	CMD_MKDIR_P = mkdir -p "$(1)"
	CMD_RM_RF = rm -rf "$(1)"
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

DEPS := $(patsubst %.c,$(DEP_ROOT_DIR)/%.d,$(ALL_C_SRCS)) \
        $(patsubst %.c,$(DEP_ROOT_DIR)/shared/%.d,$(LIBELASH_C_SRCS) $(LIBELC_C_SRCS))

.PHONY: all dirs lint clean run tests sharedlib

all: dirs $(ELC_BIN) $(LIBELASH_STATIC) $(LIBELASH_SHARED) $(LIBELC_STATIC) $(LIBELC_SHARED)

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

run: all
	$(ELC_BIN)

sharedlib: dirs $(LIBELASH_SHARED) $(LIBELC_SHARED)

-include $(DEPS)

clean:
	@$(call CMD_RM_RF,build)
	@$(call CMD_RM_RF,out)
