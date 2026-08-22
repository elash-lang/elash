TESTS_DIR     := tests
TESTS_OUT_DIR := $(OUT_DIR)/tests

ELASH_TESTS_DIR := $(TESTS_DIR)/elash
ELC_TESTS_DIR   := $(TESTS_DIR)/elc

ELASH_TESTS_SRCS := $(call rwildcard,$(ELASH_TESTS_DIR),*.c)
ELASH_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELASH_TESTS_SRCS))

ELC_TESTS_SRCS := $(call rwildcard,$(ELC_TESTS_DIR),*.c)
ELC_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELC_TESTS_SRCS))

E2E_TEST_RUNNER := $(TESTS_DIR)/e2e/runner.py

ifdef TEST_PARALLELISM
	TEST_PARALLEL_FLAG := -j$(TEST_PARALLELISM)
endif

.PHONY: test-dirs test-e2e test-elash test-elc
.PHONY: unit-test test clean-tests clean-test

test-dirs:
	@$(call CMD_MKDIR_P,$(TESTS_OUT_DIR))

$(TESTS_OUT_DIR)/elash/%$(EXE_EXT): $(TESTS_DIR)/elash/%.c $(LIBELASH_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	$(ECHO) "LD $@"
	$(Q)$(CC) $(TESTS_CFLAGS) $< $(LIBELASH_STATIC) $(TESTS_LDFLAGS) -o $@

$(TESTS_OUT_DIR)/elc/%$(EXE_EXT): $(TESTS_DIR)/elc/%.c $(LIBELC_STATIC) $(LIBELASH_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	$(ECHO) "CC $@"
	$(Q)$(CC) $(TESTS_CFLAGS) $< $(LIBELC_STATIC) $(LIBELASH_STATIC) $(TESTS_LDFLAGS) $(LLVM_LDFLAGS) -o $@

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

test-e2e: $(ELC_BIN)
	@$(call CMD_MKDIR_P,$(TESTS_OUT_DIR)/e2e)
	@echo "Running End-To-End tests"
	@$(PY) $(E2E_TEST_RUNNER) $(ELC_BIN) $(TESTS_OUT_DIR)/e2e $(TEST_PARALLEL_FLAG)

unit-test: test-elash test-elc
	@echo "All tests passed."

test: test-elash test-elc test-e2e
	@echo "All tests passed."

clean-test: clean-tests
clean-tests:
	@$(call CMD_RM_RF,$(TESTS_OUT_DIR))

FUZZ_SRC := $(TESTS_DIR)/fuzz/fuzzer.c
FUZZ_BINARY := $(TESTS_OUT_DIR)/fuzz/fuzzer$(EXE_EXT)

$(FUZZ_BINARY): $(FUZZ_SRC) $(LIBELASH_STATIC) $(LIBELC_STATIC) | test-dirs
	@$(call CMD_MKDIR_P,$(dir $@))
	@$(ECHO) "CC $@"
	$(Q)$(CC) $(TESTS_CFLAGS) $< $(LIBELASH_STATIC) $(LIBELC_STATIC) $(LDFLAGS) -o $@

.PHONY: test-fuzz-% test-fuzz

test-fuzz-%: $(FUZZ_BINARY)
	@echo "Running fuzz with count $*"
	@$(PY) $(TESTS_DIR)/fuzz/fuzz.py $(FUZZ_BINARY) $(ELC_BIN) $*

test-fuzz: $(FUZZ_BINARY)
	@$(PY) $(TESTS_DIR)/fuzz/fuzz.py $(FUZZ_BINARY) $(ELC_BIN) $(or $(FUZZ_COUNT),200)
