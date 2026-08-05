CC              ?= cc
AR              ?= ar
RM              ?= rm -f
CFLAGS          ?= -Wall -Wextra -Werror -std=gnu11 -g3
CPPFLAGS        ?=

REL_DIR         := $(shell realpath --relative-to="$(ROOT_DIR)" .)
DIR_NAME        := $(notdir $(REL_DIR))

INCLUDE_DIRS := \
	$(ROOT_DIR) \
	$(ROOT_DIR)/_internal_ \
	$(ROOT_DIR)/standards \
	$(ROOT_DIR)/files \
	$(ROOT_DIR)/interface \
	$(ROOT_DIR)/memory \
	$(ROOT_DIR)/network \
	$(ROOT_DIR)/random \
	$(ROOT_DIR)/times \
	$(ROOT_DIR)/types

QUOTE_DIRS := \
	$(wildcard $(ROOT_DIR)/*/) \
	$(wildcard $(ROOT_DIR)/*/*/) \
	$(wildcard $(ROOT_DIR)/*/*/*/)

CPPFLAGS        += $(addprefix -I,$(INCLUDE_DIRS)) $(addprefix -iquote ,$(QUOTE_DIRS)) -MMD -MP

SRC_FILES       := $(wildcard *.c) $(wildcard _internal_/*.c) $(wildcard _internal_/*/*.c)
OBJ_FILES       := $(patsubst %.c,$(BUILD_DIR)/$(REL_DIR)/%.o,$(SRC_FILES))
DEP_FILES       := $(OBJ_FILES:.o=.d)
ARCHIVE         := $(BUILD_DIR)/$(REL_DIR)/lib$(DIR_NAME).a

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

$(BUILD_DIR)/$(REL_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean-local:
	$(RM) $(OBJ_FILES) $(DEP_FILES) $(ARCHIVE)
