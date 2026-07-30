NAME := modulesmng

CC := cc

GENESIS_ROOT := ..
GENESIS_BUILD := $(GENESIS_ROOT)/build
GENESIS_LIB := $(GENESIS_BUILD)/libgenesis.a

SRC_DIR := src
OBJ_DIR := build/obj

INSTALL_DIR	:= ~/.local/bin

SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CPPFLAGS := \
	-I$(SRC_DIR) \
	-I$(GENESIS_ROOT) \
	-I$(GENESIS_ROOT)/_internal_ \
	-I$(GENESIS_ROOT)/standards \
	-I$(GENESIS_ROOT)/files \
	-I$(GENESIS_ROOT)/interface \
	-I$(GENESIS_ROOT)/memory \
	-I$(GENESIS_ROOT)/network \
	-I$(GENESIS_ROOT)/random \
	-I$(GENESIS_ROOT)/times

CFLAGS := -Wall -Wextra -Werror -g3 -O0 -MMD -MP
LDFLAGS :=
LDLIBS := $(GENESIS_LIB)

.PHONY: all genesis clean fclean re FORCE

all: $(NAME)

$(NAME): $(OBJS) $(GENESIS_LIB)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

genesis: $(GENESIS_LIB)

$(GENESIS_LIB): FORCE
	$(MAKE) -C $(GENESIS_ROOT)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r build

fclean: clean
	$(RM) $(NAME)

re: fclean all

install: all
	cp ./$(NAME) $(INSTALL_DIR)/

-include $(DEPS)
