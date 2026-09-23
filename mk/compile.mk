CC              ?= cc
AR              ?= ar
RM              ?= rm -f
CFLAGS          ?= -Wall -Wextra -Werror -std=gnu11 -g3
CPPFLAGS        ?=

REL_DIR         := $(shell realpath --relative-to="$(ROOT_DIR)" .)
DIR_NAME        := $(notdir $(REL_DIR))

# Each product sees headers from its dependencies. Lib also has temporary
# short include paths for modules still being migrated to the src/ layout.
INCLUDE_DIRS := $(ROOT_DIR)/src/core
ifneq ($(filter src/standards% src/lib%,$(REL_DIR)),)
INCLUDE_DIRS += $(ROOT_DIR)/src/standards
endif
ifneq ($(filter src/lib%,$(REL_DIR)),)
INCLUDE_DIRS += \
	$(ROOT_DIR)/src \
	$(ROOT_DIR)/src/lib \
	$(ROOT_DIR)/src/lib/files \
	$(ROOT_DIR)/src/lib/interface
endif

CPPFLAGS        += $(addprefix -I,$(INCLUDE_DIRS)) -MMD -MP

SRC_FILES       := $(wildcard *.c) $(wildcard _internal_/*.c) $(wildcard _internal_/*/*.c)
OBJ_FILES       := $(patsubst %.c,$(BUILD_DIR)/obj/$(REL_DIR)/%.o,$(SRC_FILES))
DEP_FILES       := $(OBJ_FILES:.o=.d)
ARCHIVE         := $(BUILD_DIR)/modules/$(REL_DIR)/lib$(DIR_NAME).a

.PHONY: local clean-local

ifneq ($(strip $(SRC_FILES)),)
local: $(ARCHIVE)
else
local:
	@:
endif

$(ARCHIVE): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD_DIR)/obj/$(REL_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean-local:
	$(RM) $(OBJ_FILES) $(DEP_FILES) $(ARCHIVE)

-include $(DEP_FILES)
