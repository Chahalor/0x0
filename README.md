# 0x0 - Genesis

A modular C library with reusable foundations for memory, files, interfaces,
networking, time, random values, and generic types.

## Build

Build the complete default library:

```bash
make
```

The static archive is written to `build/libgenesis.a`.

Build one main module or submodule directly with the same shared rules:

```bash
make -C memory
make -C files/json
```

Override the compiler flags when needed:

```bash
make CFLAGS='-Wall -Wextra -Werror -std=gnu11 -O2'
```

## Clean

```bash
make clean
make fclean
make re
```

## Controls

Module controls remain independent executables. For example:

```bash
interface/args/controls/controls.sh
```
