NAME            := genesis
CC              ?= cc
AR              ?= ar
RM              ?= rm -f
CFLAGS          ?= -Wall -Wextra -Werror -std=gnu11 -g3
CPPFLAGS        ?=
BUILD_DIR       := build
MODULES         := _internal_ standards files interface math memory processor random times types
TARGET          := $(BUILD_DIR)/lib$(NAME).a

.DEFAULT_GOAL := all

.PHONY: all $(MODULES) clean fclean re

all: $(TARGET)

$(TARGET): $(MODULES)
	@mkdir -p $(dir $@)
	@$(RM) $@
	@find $(BUILD_DIR) -name '*.o' -print0 | xargs -0 -r $(AR) rcs $@
	@test -f $@ || $(AR) rcs $@

$(MODULES):
	$(MAKE) -C $@ ROOT_DIR=$(CURDIR) BUILD_DIR=$(CURDIR)/$(BUILD_DIR)

clean:
	@for module in $(MODULES); do \
		$(MAKE) -C $$module ROOT_DIR=$(CURDIR) BUILD_DIR=$(CURDIR)/$(BUILD_DIR) clean; \
	done

fclean: clean
	$(RM) -r $(BUILD_DIR)

re: fclean all
