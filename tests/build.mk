TESTS_DIR     := tests
TESTS_OUT_DIR := $(OUT_DIR)/tests

ELASH_TESTS_DIR := $(TESTS_DIR)/elash
ELC_TESTS_DIR   := $(TESTS_DIR)/elc

ELASH_TESTS_SRCS := $(call rwildcard,$(ELASH_TESTS_DIR),*.c)
ELASH_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELASH_TESTS_SRCS))

ELC_TESTS_SRCS := $(call rwildcard,$(ELC_TESTS_DIR),*.c)
ELC_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELC_TESTS_SRCS))

TESTS_CFLAGS := $(CFLAGS)
TESTS_LDFLAGS := $(LDFLAGS) -lcriterion

.PHONY: test-dirs test test-elash test-elc

test-dirs:
	@$(call CMD_MKDIR_P,$(TESTS_OUT_DIR))

$(TESTS_OUT_DIR)/elash/%$(EXE_EXT): $(TESTS_DIR)/elash/%.c $(LIBELASH_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) $(TESTS_CFLAGS) $< $(LIBELASH_STATIC) $(TESTS_LDFLAGS) -o $@

$(TESTS_OUT_DIR)/elc/%$(EXE_EXT): $(TESTS_DIR)/elc/%.c $(LIBELC_STATIC) $(LIBELASH_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) $(TESTS_CFLAGS) $< $(LIBELC_STATIC) $(LIBELASH_STATIC) $(TESTS_LDFLAGS) $(LLVM_LDFLAGS) -o $@

test-elash: $(ELASH_TESTS_BINS)
	@$(foreach t,$(ELASH_TESTS_BINS), \
		echo "Running $(patsubst $(TESTS_OUT_DIR)/%,%,$(t))..." && \
		$(call FIXPATH,$(t)) && \
	) true

test-elc: $(ELC_TESTS_BINS)
	@$(foreach t,$(ELC_TESTS_BINS), \
		echo "Running $(patsubst $(TESTS_OUT_DIR)/%,%,$(t))..." && \
		$(call FIXPATH,$(t)) && \
	) true

test: test-elash test-elc
	@echo "All tests passed."
