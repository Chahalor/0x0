NAME       := genesis
CC         ?= cc
AR         ?= ar
RM         ?= rm -f
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/lib$(NAME).a

.DEFAULT_GOAL := all

.PHONY: all core standards lib test-core clean fclean re

all: $(TARGET)

core:
	$(MAKE) -C src/core

standards:
	$(MAKE) -C src/standards

lib:
	$(MAKE) -C src/lib

test-core: core
	@mkdir -p $(BUILD_DIR)/tests/core
	$(CC) -std=gnu11 -Wall -Wextra -Werror -Isrc/core \
		tests/core/error_convention.c $(BUILD_DIR)/libgenesis-core.a \
		-o $(BUILD_DIR)/tests/core/error_convention
	$(BUILD_DIR)/tests/core/error_convention
	$(CC) -std=gnu11 -Wall -Wextra -Werror -Isrc/core \
		tests/core/settings_manager.c $(BUILD_DIR)/libgenesis-core.a \
		-o $(BUILD_DIR)/tests/core/settings_manager
	$(BUILD_DIR)/tests/core/settings_manager

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
