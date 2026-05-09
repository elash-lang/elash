TESTS_DIR     := tests
TESTS_OUT_DIR := $(OUT_DIR)/tests

TESTS_SRCS := $(call rwildcard,$(TESTS_DIR),*.c)
TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(TESTS_SRCS))

TESTS_CFLAGS  := $(CFLAGS)
TESTS_LDFLAGS := $(LDFLAGS) -lcriterion $(LLVM_LDFLAGS)

.PHONY: test-dirs test

test-dirs:
	@$(call CMD_MKDIR_P,$(TESTS_OUT_DIR))

$(TESTS_OUT_DIR)/%$(EXE_EXT): $(TESTS_DIR)/%.c $(LIBELC_STATIC) $(LIBELASH_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	$(CC) $(TESTS_CFLAGS) $< $(LIBELC_STATIC) $(LIBELASH_STATIC) $(TESTS_LDFLAGS) -o $@

test: $(TESTS_BINS)
	@$(foreach t,$(TESTS_BINS), \
		echo "Running $(patsubst $(TESTS_OUT_DIR)/%,%,$(t))..." && \
		$(call FIXPATH,$(t)) && \
	) \
	echo "All tests passed."
