NAME       := genesis
AR         ?= ar
RM         ?= rm -f
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/lib$(NAME).a

.DEFAULT_GOAL := all

.PHONY: all core standards lib clean fclean re

all: $(TARGET)

core:
	$(MAKE) -C src/core

standards:
	$(MAKE) -C src/standards

lib:
	$(MAKE) -C src/lib

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
