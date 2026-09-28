ROOT_DIR  ?= ../..
BUILD_DIR ?= $(ROOT_DIR)/build

include $(ROOT_DIR)/mk/compile.mk

PRODUCT_ARCHIVE := $(BUILD_DIR)/libgenesis-$(PRODUCT).a

.DEFAULT_GOAL := all

.PHONY: all dependencies $(DEPENDENCIES) modules $(MODULES) clean

all: $(PRODUCT_ARCHIVE)

$(PRODUCT_ARCHIVE): dependencies local modules
	@mkdir -p $(dir $@)
	@$(RM) $@
	@if test -d $(BUILD_DIR)/obj/$(REL_DIR); then \
		find $(BUILD_DIR)/obj/$(REL_DIR) -name '*.o' -print0 | xargs -0 -r $(AR) rcs $@; \
	fi
	@test -f $@ || $(AR) rcs $@

dependencies: $(DEPENDENCIES)

$(DEPENDENCIES):
	$(MAKE) -C ../$@

modules: $(MODULES)

$(MODULES):
	$(MAKE) -C $@

clean: clean-local
	@for module in $(MODULES); do \
		$(MAKE) -C $$module clean; \
	done
	$(RM) $(PRODUCT_ARCHIVE)
