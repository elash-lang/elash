include make/config.mk

SRC_DIR     := src
TEST_DIR    := tests
INCLUDE_DIR := include
SCRIPTS_DIR := scripts

OBJ_ROOT_DIR := build/$(BUILD)/obj
DEP_ROOT_DIR := build/$(BUILD)/dep

OUT_DIR     := out/$(BUILD)
LIB_DIR     := $(OUT_DIR)/lib
BIN_DIR     := $(OUT_DIR)/bin

EXE_EXT :=
ifeq ($(PLATFORM),windows)
	EXE_EXT    := .exe
	SHARED_EXT := .dll
	STATIC_EXT := .a
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

DIST_NAME := $(OUT_DIR)/elash-$(VERSION)-$(DIST_OS)-$(DIST_ARCH)-$(BUILD)
