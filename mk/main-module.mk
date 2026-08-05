ROOT_DIR        ?= ..
BUILD_DIR       ?= $(ROOT_DIR)/build

include $(ROOT_DIR)/mk/compile.mk

.DEFAULT_GOAL := all

SUBMODULES      ?= $(patsubst %/,%,$(sort $(dir $(wildcard */makefile */Makefile))))

.PHONY: all submodules $(SUBMODULES) clean

all: local submodules

submodules: $(SUBMODULES)

$(SUBMODULES):
	$(MAKE) -C $@

clean: clean-local
	@for submodule in $(SUBMODULES); do \
		$(MAKE) -C $$submodule clean; \
	done
