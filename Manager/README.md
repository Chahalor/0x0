# 0x0 module manager

The Manager creates a self-contained 0x0 source export containing only the
requested modules and their declared dependencies.

## Build

```bash
make -C Manager
```

## Setup an export

```bash
genesis \
  --output ./vendor/0x0 \
  --cache-dir ~/.cache/genesis \
  setup files
```

The destination receives:

- `genesis.h`, containing the selected public module headers;
- `GENESIS.toml`, containing metadata only for the resolved module set;
- a root `Makefile` producing `build/libgenesis.a`;
- shared build rules under `mk/`;
- one regenerated `makefile` per selected main module and submodule.

Nested modules are assembled from the resolved hierarchy. Unselected sibling
modules are not copied. The Manager validates all selected source directories
and public headers before writing the export, so an incomplete cache fails
without leaving a partial source tree.

Build a generated export with:

```bash
make -C ./vendor/0x0
```

## Metadata

The cache manifest is `GENESIS.toml`. Each module table defines its path,
public/private headers, dependencies, tags, defines, and controls. Dependencies
must include every module required by the implementation or public headers.
