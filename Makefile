NAME := genesis
AR ?= ar
RM ?= rm -f
BUILD_DIR := build
TARGET := $(BUILD_DIR)/lib$(NAME).a
ARCHIVE_STAGE := $(BUILD_DIR)/archive-objects
MODULE_DIRS := $(patsubst %/,%,$(dir $(shell find src/core src/standards src/lib -name makefile -print)))
MODULE_SOURCES := $(foreach dir,$(MODULE_DIRS),$(wildcard \
	$(dir)/*.c $(dir)/_internal_/*.c $(dir)/_internal_/*/*.c))
MODULE_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(MODULE_SOURCES))

.DEFAULT_GOAL := all
.PHONY: all clean core fclean lib norm re standard standards test

all: $(TARGET)

core:
	$(MAKE) -C src/core

standard:
	$(MAKE) -C src/standards

standards: standard

lib:
	$(MAKE) -C src/lib

$(TARGET): lib
	@mkdir -p $(dir $@)
	@$(RM) -r $(ARCHIVE_STAGE)
	@mkdir -p $(ARCHIVE_STAGE)
	@set -e; for object in $(MODULE_OBJECTS); do \
		name=$$(printf '%s' "$${object#$(BUILD_DIR)/obj/}" | tr / @); \
		ln -s "$(CURDIR)/$$object" "$(ARCHIVE_STAGE)/$$name"; \
	done
	@$(RM) $@
	$(AR) rcs $@ $(ARCHIVE_STAGE)/*.o

test:
	$(MAKE) -C tests test

norm:
	$(MAKE) -C tests norm

clean:
	$(MAKE) -C tests clean
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/standards clean
	$(MAKE) -C src/core clean
	$(RM) -r $(ARCHIVE_STAGE)
	$(RM) $(TARGET)

fclean: clean
	$(RM) -r $(BUILD_DIR)

re: fclean all
