ROOT_DIR        ?= ../..
BUILD_DIR       ?= $(ROOT_DIR)/build

include $(ROOT_DIR)/mk/compile.mk

.DEFAULT_GOAL := all

.PHONY: all clean

all: local

clean: clean-local
