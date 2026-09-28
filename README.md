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
The `core` product exposes allocator, buffer, setting, and error APIs through
`src/core/core.h`.

Core functions returning a status use `0` for success, positive `errno` values
for standard errors, and negative values for module errors. Pointer-returning
functions use `NULL` when no pointer can be returned. Each module has a
`core_<module>_strerror()` function and an error descriptor;
`genesis_strerror(&descriptor, code)` selects the appropriate one. Module
enums provide nonnegative message-table indices;
functions return their negation for module errors. For example:

```c
int status = core_buffer_append(&buffer, bytes, length);
if (status != 0)
    fprintf(stderr, "%s\n",
        genesis_strerror(&genesis_buffer_error_module, status));
```

Run the core and reviewed lib contract suites with `make test`. The suite uses
CMocka, AddressSanitizer, UndefinedBehaviorSanitizer, Valgrind and Cppcheck.
Individual checks and prerequisites are documented in
[docs/testing.md](docs/testing.md).

The core settings manager lets modules expose configuration keys through their
own `setting.h`. Use `genesis_settings_set/get/reset` on a caller-owned
`t_genesis_settings` object. Allocator and buffer growth each have a module
key; set/get calls require the actual object size. Buffers consume both through
`core_buffer_new_with_settings`. See
[docs/settings.md](docs/settings.md) for usage and module integration.

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
