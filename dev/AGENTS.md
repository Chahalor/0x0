# Genesis implementation rules

These rules apply when designing, implementing, reviewing, or fixing Genesis.
Read `docs/norm.md` before changing code. It is the authority for architecture,
formatting, documentation, settings, errors, and tests. Use `docs/testing.md`
for test commands and `docs/settings.md` for configuration and ownership.
When a rule here and a referenced document differ, follow `docs/norm.md` and
update this file to match it.

## Architecture and public API

- Treat each directory directly under `src/` as a product with a limited,
  documented public API. Keep module implementation inside its `_internal_`
  directory; other modules must depend on its public API only.
- Put a module's public functions in its module header, exported types in
  `types.h`, settings in `setting.h`, and error declarations in `error.h` when
  needed. Do not expose private types or functions in public headers.
- Keep public C entry points thin: validate arguments and delegate behavior to
  private functions. Follow the file and function layout in `docs/norm.md`.
- Document every public function with Doxygen: accepted inputs, outputs,
  return and error codes, state changes, ownership, lifetime, boundaries, and
  at least one usage example. State what happens on failure.
- Preserve public compatibility unless an API change is explicitly part of the
  task. Update callers and documentation together with any approved change.

## Configuration, allocation, and errors

- A configurable behavior module exposes a public settings struct, global instance,
  and no-argument apply function. The struct contains an `apply` function
  pointer. Apply validates and publishes a complete private configuration.
  Runtime calls read only that private configuration and take no settings
  argument. See `docs/settings.md`.
- Keep `core/setting` focused on values shared globally across Genesis. Its
  allocator callbacks take effect directly. Each module owns its public
  behavior settings, apply logic, and private runtime configuration.
- Use lib/memory for ordinary dynamic values and release them with mem_free.
  Internal opaque structures may use the current global allocator; release
  each allocation with the allocator that created it. Check size arithmetic
  for overflow before allocating or copying.
- The memory module reads `G_SETTINGS.allocator` directly for each allocation
  and release. Do not cache allocator callbacks in its manager or arenas.
- Follow the shared error convention: `0` means success; positive codes are
  standard `errno` values; negative codes are module-specific. Pointer results
  use `NULL` on failure and set `errno` when applicable.
- Each module error enum starts with `MODULE_ERROR_SUCCESS = 0` and ends with
  `MODULE_ERROR_UNKNOWN`. Provide a module `strerror` function backed by a
  `const char *const` array with designated error-code indices, plus a
  `t_genesis_error_module` descriptor for `genesis_strerror`.
- Never use an `if` or `switch` chain to select module error messages. Define
  the meaning of invalid arguments, missing data, and partial work in each
  public contract.

## Test-first implementation

1. Define or revise public headers and the contract of each affected public
   function before implementing it.
2. Write CMocka tests for normal behavior, invalid inputs, error codes,
   defaults, ownership, and boundary cases. For a bug, first add a test that
   fails on the old behavior; keep it as a regression test.
3. Add the module to the appropriate Make test targets. Only then implement
   the behavior and make the tests pass.
4. Run the relevant module target, then `make test`: strict C11 compilation,
   AddressSanitizer and UndefinedBehaviorSanitizer, Valgrind Memcheck, and
   Cppcheck. Report any check that cannot run and why.

Do not weaken tests, suppress diagnostics without a documented reason, or
change a public contract merely to make a failing implementation pass. Keep
failure paths safe: no lost allocation, stale pointer, partially updated
output, or silent error unless the documented contract explicitly allows it.

## C style and work in progress

- Follow the naming, alignment, scope, and file-layout rules in `docs/norm.md`.
  Functions with no parameters use `(void)`; every value-returning statement
  uses `return (expression);`.
- Keep unrelated worktree changes intact. Inspect the diff before editing and
  report the files changed and the tests run when the task is complete.
