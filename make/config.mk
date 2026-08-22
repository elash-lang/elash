ifndef ELASH_CONFIG
ELASH_CONFIG :=

########## the toolchain ###########
CC ?= cc
AR ?= ar
PY ?= python3
BUILD ?= release

###### platform & shell detection #######
IS_POSIX_SHELL := $(shell echo posix 2>/dev/null)

ifeq ($(IS_POSIX_SHELL),posix)
	USE_POSIX_SHELL := yes
else
	USE_POSIX_SHELL := no
endif

ifeq ($(OS),Windows_NT)
	PLATFORM := windows
else
	PLATFORM := posix
endif

###### compiler kind detection ######
CC_VERSION_OUTPUT := $(shell $(CC) --version)

ifeq ($(findstring clang,$(CC_VERSION_OUTPUT)),clang)
    CCKIND := clang
else ifeq ($(findstring GCC,$(CC_VERSION_OUTPUT)),GCC)
    CCKIND := gcc
else ifeq ($(findstring icc,$(CC_VERSION_OUTPUT)),icc)
    CCKIND := intel
else
    CCKIND := unknown
endif

########## version ###########
ifeq ($(wildcard VERSION),)
$(error VERSION file not found)
endif

ifeq ($(USE_POSIX_SHELL),yes)
	VERSION   := $(strip $(shell cat VERSION))
	DIST_OS   := $(if $(filter windows,$(PLATFORM)),windows,$(shell uname -s | tr '[:upper:]' '[:lower:]'))
	DIST_ARCH := $(shell uname -m | tr '[:upper:]' '[:lower:]')
else
	VERSION   := $(strip $(shell type VERSION))
	DIST_OS   := windows
	DIST_ARCH := $(shell powershell -NoProfile -Command "$$env:PROCESSOR_ARCHITECTURE.ToLower()")
endif

########### flags ###########
CSTD     := -std=c11
PIC_CFLAGS := -fPIC

ifneq ($(CCKIND),unknown)
	WARNINGS := \
		-Wall -Wextra \
		-Werror=implicit-fallthrough -Werror=switch \
		-Werror=uninitialized -Werror=return-type \
		-Werror=write-strings -Werror=undef
else
	WARNINGS :=
endif
ifeq ($(CCKIND),gcc)
	# seems to only be supported by gcc (clang does't recognize it)
	WARNINGS += -Werror=discarded-qualifiers
endif

COMMON_CFLAGS := $(CSTD) $(WARNINGS) -I$(INCLUDE_DIR)

ifeq ($(BUILD),debug)
	DEFAULT_CFLAGS := $(COMMON_CFLAGS) -O0 -g -DEL_DEBUG
	DEFAULT_LDFLAGS :=
else ifeq ($(BUILD),debug-san)
	DEFAULT_CFLAGS := $(COMMON_CFLAGS) -O0 -g -DEL_DEBUG -fsanitize=address,undefined
	DEFAULT_LDFLAGS := -fsanitize=address,undefined
else ifeq ($(BUILD),release)
	DEFAULT_CFLAGS := $(COMMON_CFLAGS) -O3 -DNDEBUG
	DEFAULT_LDFLAGS :=
else ifeq ($(BUILD),rel-debug)
	DEFAULT_CFLAGS := $(COMMON_CFLAGS) -O3 -g -DNDEBUG
	DEFAULT_LDFLAGS :=
else ifeq ($(BUILD),rel-debug-san)
	DEFAULT_CFLAGS := $(COMMON_CFLAGS) -O3 -g -DNDEBUG -fsanitize=address,undefined
	DEFAULT_LDFLAGS := -fsanitize=address,undefined
else ifeq ($(BUILD),manual)
	DEFAULT_CFLAGS  := $(COMMON_CFLAGS)
	DEFAULT_LDFLAGS :=
else
	$(error Unknown BUILD=$(BUILD))
endif

EXTRA_CFLAGS  ?=
EXTRA_LDFLAGS ?=

EXTRA_TESTS_CFLAGS  ?=
EXTRA_TESTS_LDFLAGS ?=

CRITERION ?= -lcriterion

CFLAGS  ?= $(DEFAULT_CFLAGS) $(EXTRA_CFLAGS)
LDFLAGS ?= $(DEFAULT_LDFLAGS) $(EXTRA_LDFLAGS)

TESTS_CFLAGS  ?= $(DEFAULT_CFLAGS) $(EXTRA_TESTS_CFLAGS)
TESTS_LDFLAGS ?= $(DEFAULT_LDFLAGS) $(EXTRA_TESTS_LDFLAGS) $(CRITERION)

######## llvm configuration #########
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

###### platform specific commands abstraction ####
ifeq ($(USE_POSIX_SHELL),yes)
	CMD_MKDIR_P = mkdir -p "$(1)"
	CMD_RM_RF   = rm -rf "$(1)"
	CMD_RM_F    = rm -f "$(1)"
	DIST_FILE  := $(DIST_NAME).tar.gz
	CMD_ARCHIVE = tar -czf $(DIST_FILE) -C $(OUT_DIR) bin lib
	FIXPATH     = $(1)
else
	CMD_MKDIR_P = powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(subst /,\,$(1))' | Out-Null"
	CMD_RM_RF   = powershell -NoProfile -Command "Remove-Item -Recurse -Force -Path '$(subst /,\,$(1))' | Out-Null"
	CMD_RM_F    = powershell -NoProfile -Command "Remove-Item -Force -Path '$(subst /,\,$(1))' | Out-Null"
	DIST_FILE  := $(DIST_NAME).zip
	CMD_ARCHIVE = Compress-Archive -Path '$(subst /,\,$(OUT_DIR)/*)' -DestinationPath '$(subst /,\,$(DIST_FILE))' -Force
	FIXPATH     = $(subst /,\,$(1))
endif

############### logging #################
# for nicer output
ifeq ($(V),1)
    Q =
    ifeq ($(OS),Windows_NT)
        ECHO = @rem
    else
        ECHO = @:
    endif
else
    Q = @
    ECHO = @echo
endif

############ useful "functions" ##########
rwildcard = \
	$(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) \
	$(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

endif
