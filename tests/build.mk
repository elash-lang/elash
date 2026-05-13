PYTHON ?= python

TESTS_DIR     := tests
TESTS_OUT_DIR := $(OUT_DIR)/tests

ELASH_TESTS_DIR := $(TESTS_DIR)/elash
ELC_TESTS_DIR   := $(TESTS_DIR)/elc

ELASH_TESTS_SRCS := $(call rwildcard,$(ELASH_TESTS_DIR),*.c)
ELASH_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELASH_TESTS_SRCS))

ELC_TESTS_SRCS := $(call rwildcard,$(ELC_TESTS_DIR),*.c)
ELC_TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(ELC_TESTS_SRCS))

E2E_TEST_RUNNER := $(TESTS_DIR)/e2e/runner.py

TESTS_CFLAGS := $(CFLAGS)
TESTS_LDFLAGS := $(LDFLAGS) -lcriterion

.PHONY: test-dirs test-e2e test-elash test-elc
.PHONY: unit-test test

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

test-e2e: $(ELC_BIN)
	@$(call CMD_MKDIR_P,$(TESTS_OUT_DIR)/e2e)
	@echo "Running End-To-End tests"
	@$(PYTHON) $(E2E_TEST_RUNNER) $(ELC_BIN) $(TESTS_OUT_DIR)/e2e

unit-test: test-elash test-elc
	@echo "All tests passed."

test: test-elash test-elc test-e2e
	@echo "All tests passed."
