NAME       := genesis
CC         ?= cc
AR         ?= ar
RM         ?= rm -f
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/lib$(NAME).a

.DEFAULT_GOAL := all

TEST_GROUPS := allocator settings buffer errors
TEST_SOURCES := $(wildcard src/core/*/*.c src/core/*/_internal_/*.c \
	src/core/*/_internal_/*/*.c)
MEMORY_SOURCES := $(wildcard src/lib/memory/*.c src/lib/memory/_internal_/*.c)
HASH_MAP_SOURCES := src/lib/types/types.c $(wildcard src/lib/types/_internal_/map/*.c)
UTILITIES_SOURCES := src/lib/random/random.c src/lib/times/convert.c
LOG_SOURCES := src/lib/interface/log/log.c $(wildcard src/lib/interface/log/_internal_/*.c)
TEST_CFLAGS := -std=c11 -pedantic -Wall -Wextra -Werror -Wstrict-prototypes -g3
TEST_CPPFLAGS := -Isrc -Isrc/core -Isrc/lib -Isrc/standards $(CMOCKA_CFLAGS)
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
HASH_MAP_TEST_BIN := $(BUILD_DIR)/tests/lib/hash_map
HASH_MAP_SANITIZER_BIN := $(HASH_MAP_TEST_BIN).san
UTILITIES_TEST_BIN := $(BUILD_DIR)/tests/lib/utilities
UTILITIES_SANITIZER_BIN := $(UTILITIES_TEST_BIN).san
LOG_TEST_BIN := $(BUILD_DIR)/tests/lib/log
LOG_SANITIZER_BIN := $(LOG_TEST_BIN).san

.PHONY: all core standards lib test test-core test-strict test-sanitize \
	test-valgrind test-cppcheck test-deps test-memory test-memory-bucket test-hash-map \
	test-utilities test-log \
	$(addprefix test-,$(TEST_GROUPS)) \
	clean fclean re

all: $(TARGET)

core:
	$(MAKE) -C src/core

standards:
	$(MAKE) -C src/standards

lib:
	$(MAKE) -C src/lib

test:
	$(MAKE) test-strict
	$(MAKE) test-sanitize
	$(MAKE) test-valgrind
	$(MAKE) test-cppcheck

test-core: test-allocator test-settings test-buffer test-errors

test-deps:
	@test -n "$(CMOCKA_LIBS)" || { \
		echo 'CMocka is required: install libcmocka-dev or set CMOCKA_CFLAGS and CMOCKA_LIBS'; \
		exit 1; \
	}

$(BUILD_DIR)/tests/core/%: tests/core/%.c $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(BUILD_DIR)/tests/core/%.san: tests/core/%.c $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_TEST_BIN): tests/lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_SANITIZER_BIN): tests/lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(MEMORY_BUCKET_TEST_BIN): tests/lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(MEMORY_BUCKET_SANITIZER_BIN): tests/lib/memory.c $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -DMEM_MANAGER_TYPE=0 -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(HASH_MAP_TEST_BIN): tests/lib/hash_map.c $(HASH_MAP_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(HASH_MAP_SANITIZER_BIN): tests/lib/hash_map.c $(HASH_MAP_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(UTILITIES_TEST_BIN): tests/lib/utilities.c $(UTILITIES_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(UTILITIES_SANITIZER_BIN): tests/lib/utilities.c $(UTILITIES_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -fno-omit-frame-pointer \
		-fsanitize=address,undefined $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

$(LOG_TEST_BIN): tests/lib/log.c $(LOG_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L \
		-Isrc/lib/interface $(TEST_CPPFLAGS) $^ $(CMOCKA_LIBS) -o $@

$(LOG_SANITIZER_BIN): tests/lib/log.c $(LOG_SOURCES) $(MEMORY_SOURCES) $(TEST_SOURCES) | test-deps
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -D_POSIX_C_SOURCE=200809L \
		-fno-omit-frame-pointer -fsanitize=address,undefined \
		-Isrc/lib/interface $(TEST_CPPFLAGS) $^ \
		$(CMOCKA_LIBS) -fsanitize=address,undefined -o $@

test-allocator: $(BUILD_DIR)/tests/core/allocator
	$<

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

test-hash-map: $(HASH_MAP_TEST_BIN)
	$<

test-utilities: $(UTILITIES_TEST_BIN)
	$<

test-log: $(LOG_TEST_BIN)
	$<

test-strict: test-allocator test-settings test-buffer test-errors test-memory test-memory-bucket test-hash-map test-utilities test-log

test-sanitize: $(SANITIZER_BINS) $(MEMORY_SANITIZER_BIN) $(MEMORY_BUCKET_SANITIZER_BIN) $(HASH_MAP_SANITIZER_BIN) $(UTILITIES_SANITIZER_BIN) $(LOG_SANITIZER_BIN)
	@set -e; for binary in $(SANITIZER_BINS) $(MEMORY_SANITIZER_BIN) $(MEMORY_BUCKET_SANITIZER_BIN) $(HASH_MAP_SANITIZER_BIN) $(UTILITIES_SANITIZER_BIN) $(LOG_SANITIZER_BIN); do \
		ASAN_OPTIONS='$(ASAN_OPTIONS)' \
		UBSAN_OPTIONS='$(UBSAN_OPTIONS)' $$binary; \
	done

test-valgrind: $(TEST_BINS) $(MEMORY_TEST_BIN) $(MEMORY_BUCKET_TEST_BIN) $(HASH_MAP_TEST_BIN) $(UTILITIES_TEST_BIN) $(LOG_TEST_BIN)
	@set -e; for binary in $(TEST_BINS) $(MEMORY_TEST_BIN) $(MEMORY_BUCKET_TEST_BIN) $(HASH_MAP_TEST_BIN) $(UTILITIES_TEST_BIN) $(LOG_TEST_BIN); do \
		$(VALGRIND) --quiet --leak-check=full --show-leak-kinds=all \
			--errors-for-leak-kinds=all --error-exitcode=1 $$binary; \
	done

test-cppcheck:
	$(CPPCHECK) --enable=warning,style,performance,portability \
		--error-exitcode=1 --inline-suppr \
		--suppress=unsignedLessThanZero -Isrc/core src/core

$(TARGET): lib
	@mkdir -p $(dir $@)
	@$(RM) $@
	@find $(BUILD_DIR)/obj/src -name '*.o' -print0 | xargs -0 -r $(AR) rcs $@
	@test -f $@ || $(AR) rcs $@

clean:
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/standards clean
	$(MAKE) -C src/core clean

fclean: clean
	$(RM) -r $(BUILD_DIR)

re: fclean all
