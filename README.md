# 0x0 - Genesis

A modular C library organized into three products under `src/`:

- `core`: common error, setting, allocator, and buffer foundations;
- `standards`: shared headers, built after `core`;
- `lib`: reusable C modules, built after `standards` and `core`.

## Build

Build the complete default library:

```bash
make
```

The combined static archive is written to `build/libgenesis.a`.

Build a product separately:

```bash
make core
make standards
make lib
```

You can also build from a product directory, for example `make -C src/core`.
The products produce `build/libgenesis-core.a`,
`build/libgenesis-standards.a`, and `build/libgenesis-lib.a` respectively.
Building `standards` also builds `core`; building `lib` builds both dependencies.
The four `core` modules are templates and do not expose an API yet.

Link a program against the combined archive with headers from the product
directories:

```bash
cc -Isrc -Isrc/lib -Isrc/standards app.c build/libgenesis.a -o app
```

When linking the separate product archives, place them in dependency order:
`build/libgenesis-lib.a`, `build/libgenesis-standards.a`, then
`build/libgenesis-core.a`.

Build one main module or submodule directly with the same shared rules:

```bash
make -C src/lib/memory
make -C src/lib/files/json
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
