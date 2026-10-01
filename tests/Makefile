CC ?= cc
AR ?= ar
RM ?= rm -f
BUILD_DIR := ../build
TARGET := $(BUILD_DIR)/libgenesis.a
MODULE_DIRS := $(patsubst %/,%,$(dir $(shell find ../src/core ../src/standards ../src/lib -name makefile -print)))
MODULE_SOURCES := $(foreach dir,$(MODULE_DIRS),$(wildcard \
	$(dir)/*.c $(dir)/_internal_/*.c $(dir)/_internal_/*/*.c))
MODULE_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/obj/src/%.o,$(patsubst ../src/%,%,$(MODULE_SOURCES)))

TEST_GROUPS := settings buffer errors
TEST_SOURCES := $(wildcard ../src/core/*/*.c ../src/core/*/_internal_/*.c \
	../src/core/*/_internal_/*/*.c)
MEMORY_SOURCES := $(wildcard ../src/lib/memory/*.c ../src/lib/memory/_internal_/*.c)
UTILITIES_SOURCES := ../src/lib/random/random.c ../src/lib/times/convert.c
FORMAT_SOURCES := $(wildcard ../src/lib/interface/format/*.c ../src/lib/interface/format/_internal_/*.c)
FORMAT_HEADERS := $(wildcard ../src/lib/interface/format.h ../src/lib/interface/format/*.h ../src/lib/interface/format/_internal_/*.h)
UUID_SOURCES := $(wildcard ../src/lib/types/uuid/*.c ../src/lib/types/uuid/_internal_/*.c)
UUID_HEADERS := $(wildcard ../src/lib/types/uuid.h ../src/lib/types/uuid/*.h ../src/lib/types/uuid/_internal_/*.h)
LOG_SOURCES := ../src/lib/interface/log/log.c $(wildcard ../src/lib/interface/log/_internal_/*.c)
TEST_CFLAGS := -std=c11 -pedantic -Wall -Wextra -Werror -Wstrict-prototypes -g3
TEST_CPPFLAGS := -I../src -I../src/core -I../src/lib -I../src/standards $(CMOCKA_CFLAGS)
CMOCKA_CFLAGS ?= $(shell pkg-config --cflags cmocka 2>/dev/null)
CMOCKA_LIBS ?= $(shell pkg-config --libs cmocka 2>/dev/null)
CPPCHECK ?= cppcheck
VALGRIND ?= valgrind
ASAN_OPTIONS ?= detect_leaks=1:halt_on_error=1
UBSAN_OPTIONS ?= halt_on_error=1:print_stacktrace=1
TEST_BINS := $(addprefix $(BUILD_DIR)/tests/core/,$(TEST_GROUPS))
SANITIZER_BINS := $(addsuffix .san,$(TEST_BINS))
MEMORY_TEST_BIN := $(BUILD_DIR)/tests/lib/memory
MEMORY_SANITIZER_BIN := $(MEMORY_TEST_BIN).san
MEMORY_BUCKET_TEST_BIN := $(BUILD_DIR)/tests/lib/memory.bucket
MEMORY_BUCKET_SANITIZER_BIN := $(MEMORY_BUCKET_TEST_BIN).san
MEMORY_ARENA_TEST_BIN := $(BUILD_DIR)/tests/lib/memory_arena
MEMORY_ARENA_SANITIZER_BIN := $(MEMORY_ARENA_TEST_BIN).san
MEMORY_ARENA_BUCKET_TEST_BIN := $(BUILD_DIR)/tests/lib/memory_arena.bucket
MEMORY_ARENA_BUCKET_SANITIZER_BIN := $(MEMORY_ARENA_BUCKET_TEST_BIN).san
UTILITIES_TEST_BIN := $(BUILD_DIR)/tests/lib/utilities
UTILITIES_SANITIZER_BIN := $(UTILITIES_TEST_BIN).san
LOG_TEST_BIN := $(BUILD_DIR)/tests/lib/log
LOG_SANITIZER_BIN := $(LOG_TEST_BIN).san
UUID_TEST_BIN := $(BUILD_DIR)/tests/lib/uuid
UUID_SANITIZER_BIN := $(UUID_TEST_BIN).san
FORMAT_TEST_BIN := $(BUILD_DIR)/tests/lib/format
FORMAT_SANITIZER_BIN := $(FORMAT_TEST_BIN).san

.PHONY: test norm test-core test-strict test-sanitize \
	test-valgrind test-cppcheck test-deps test-memory test-memory-bucket test-memory-arena test-memory-arena-bucket \
	test-utilities test-log test-format test-uuid test-archive clean \
	$(addprefix test-,$(TEST_GROUPS))

.DEFAULT_GOAL := test

test:
	$(MAKE) test-strict
	$(MAKE) test-sanitize
	$(MAKE) test-valgrind
	$(MAKE) test-cppcheck

test-core: test-settings test-buffer test-errors

test-deps:
	@test -n "$(CMOCKA_LIBS)" || { \
		echo 'CMocka is required: install libcmocka-dev or set CMOCKA_CFLAGS and CMOCKA_LIBS'; \
		exit 1; \
	}

$(BUILD_DIR)/tests/core/%: core/%.c $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(BUILD_DIR)/tests/core/%.san: core/%.c $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_TEST_BIN): lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_SANITIZER_BIN): lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_BUCKET_TEST_BIN): lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_BUCKET_SANITIZER_BIN): lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_ARENA_TEST_BIN): lib/memory_arena.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_ARENA_SANITIZER_BIN): lib/memory_arena.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_ARENA_BUCKET_TEST_BIN): lib/memory_arena.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_ARENA_BUCKET_SANITIZER_BIN): lib/memory_arena.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(UTILITIES_TEST_BIN): lib/utilities.c $(UTILITIES_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(UTILITIES_SANITIZER_BIN): lib/utilities.c $(UTILITIES_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(LOG_TEST_BIN): lib/log.c $(LOG_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L \
		-I../src/lib/interface $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(LOG_SANITIZER_BIN): lib/log.c $(LOG_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L \
		-fno-omit-frame-pointer -fsanitize=address,undefined \
		-I../src/lib/interface $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(UUID_TEST_BIN): lib/uuid.c $(UUID_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) $(UUID_HEADERS) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L $(TEST_CPPFLAGS) $(filter %.c,$^) $(CMOCKA_LIBS) -o $@

$(UUID_SANITIZER_BIN): lib/uuid.c $(UUID_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) $(UUID_HEADERS) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $(filter %.c,$^) $(CMOCKA_LIBS) \
		-fsanitize=address,undefined -o $@

$(FORMAT_TEST_BIN): lib/format.c $(FORMAT_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) $(FORMAT_HEADERS) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $(filter %.c,$^) $(CMOCKA_LIBS) -o $@

$(FORMAT_SANITIZER_BIN): lib/format.c $(FORMAT_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) $(FORMAT_HEADERS) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer -fsanitize=address,undefined $(TEST_CPPFLAGS) $(filter %.c,$^) $(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

test-settings: $(BUILD_DIR)/tests/core/settings
	$<

test-buffer: $(BUILD_DIR)/tests/core/buffer
	$<

test-errors: $(BUILD_DIR)/tests/core/errors
	$<

test-memory: $(MEMORY_TEST_BIN)
	$<

test-memory-bucket: $(MEMORY_BUCKET_TEST_BIN)
	$<

test-memory-arena: $(MEMORY_ARENA_TEST_BIN)
	$<

test-memory-arena-bucket: $(MEMORY_ARENA_BUCKET_TEST_BIN)
	$<

test-utilities: $(UTILITIES_TEST_BIN)
	$<

test-log: $(LOG_TEST_BIN)
	$<

test-format: $(FORMAT_TEST_BIN)
	$<

test-uuid: $(UUID_TEST_BIN)
	$<

test-strict: test-settings test-buffer test-errors test-memory test-memory-bucket test-memory-arena test-memory-arena-bucket test-utilities test-log test-format test-uuid

test-sanitize: $(SANITIZER_BINS) $(MEMORY_SANITIZER_BIN) $(MEMORY_BUCKET_SANITIZER_BIN) $(MEMORY_ARENA_SANITIZER_BIN) $(MEMORY_ARENA_BUCKET_SANITIZER_BIN) $(UTILITIES_SANITIZER_BIN) $(LOG_SANITIZER_BIN) $(FORMAT_SANITIZER_BIN) $(UUID_SANITIZER_BIN)
	@set -e; for binary in $(SANITIZER_BINS) $(MEMORY_SANITIZER_BIN) $(MEMORY_BUCKET_SANITIZER_BIN) $(MEMORY_ARENA_SANITIZER_BIN) $(MEMORY_ARENA_BUCKET_SANITIZER_BIN) $(UTILITIES_SANITIZER_BIN) $(LOG_SANITIZER_BIN) $(FORMAT_SANITIZER_BIN) $(UUID_SANITIZER_BIN); do \
		ASAN_OPTIONS='$(ASAN_OPTIONS)' \
		UBSAN_OPTIONS='$(UBSAN_OPTIONS)' $$binary; \
	done

test-valgrind: $(TEST_BINS) $(MEMORY_TEST_BIN) $(MEMORY_BUCKET_TEST_BIN) $(MEMORY_ARENA_TEST_BIN) $(MEMORY_ARENA_BUCKET_TEST_BIN) $(UTILITIES_TEST_BIN) $(LOG_TEST_BIN) $(FORMAT_TEST_BIN) $(UUID_TEST_BIN)
	@set -e; for binary in $(TEST_BINS) $(MEMORY_TEST_BIN) $(MEMORY_BUCKET_TEST_BIN) $(MEMORY_ARENA_TEST_BIN) $(MEMORY_ARENA_BUCKET_TEST_BIN) $(UTILITIES_TEST_BIN) $(LOG_TEST_BIN) $(FORMAT_TEST_BIN) $(UUID_TEST_BIN); do \
		$(VALGRIND) --quiet --leak-check=full --show-leak-kinds=all \
			--errors-for-leak-kinds=all --error-exitcode=1 $$binary; \
	done

test-cppcheck:
	$(CPPCHECK) --enable=warning,style,performance,portability \
		--error-exitcode=1 --inline-suppr \
		--suppress=unsignedLessThanZero -I../src/core -I../src ../src/core ../src/lib/interface/format ../src/lib/memory ../src/lib/types/uuid

norm: test-cppcheck

test-archive:
	$(MAKE) -C .. all
	@test $(words $(MODULE_OBJECTS)) -eq $$($(AR) t $(TARGET) | wc -l)
	@test -z "$$($(AR) t $(TARGET) | sort | uniq -d)"

clean:
	$(RM) -r $(BUILD_DIR)/tests
