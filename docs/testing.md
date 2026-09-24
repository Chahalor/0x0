# Tests

## Test-first module workflow

The module skeleton is complete before implementation starts. First write its
public headers (`types.h`, the module API header, and `setting.h` or `error.h`
when needed). Specify the contract of each public function in the API docs:
accepted arguments, results and error codes, effects on state, ownership,
defaults and limits. Then write CMocka tests for those contracts and add the
module to the relevant Make targets. An initially failing test is expected at
this stage. Implement the module only after the API, contracts and tests are
reviewable together.

For a behavior bug, add a CMocka test that fails on the old behavior before
fixing it. Keep the test afterward. For a public API change, update the
contract and its tests first. This workflow also applies when existing modules
are revised.

## Current suite

The current CMocka contract suite covers the public functions in the four
`core` modules: allocator, setting, buffer and error. Tests for the other
Genesis products will be added as their public contracts are stabilized.

Install CMocka development headers and library, Valgrind, and Cppcheck. A C11
compiler with AddressSanitizer and UndefinedBehaviorSanitizer support is also
required. Then run:

```sh
make test
```

`make test` runs these checks in order:

| Target | Check |
| --- | --- |
| `test-strict` | Build all core sources and CMocka tests with C11, pedantic warnings, and `-Werror`; run contract tests. |
| `test-sanitize` | Run the same tests with AddressSanitizer and UndefinedBehaviorSanitizer. |
| `test-valgrind` | Run the strict binaries under Memcheck, failing on errors or leaks. |
| `test-cppcheck` | Analyze all core sources with Cppcheck warnings, style, performance and portability checks. |

`make test-core` is an alias for `test-strict`. To run one contract group, use
`make test-allocator`, `make test-settings`, `make test-buffer` or
`make test-errors`.

The tests exercise normal results, invalid arguments, error codes, settings
defaults and validation, allocator ownership, buffer growth and boundary
behavior. Regression cases cover buffer self-append during reallocation,
internal source ranges outside the logical length, and reset of settings
entries at the head, middle and tail of the list. Strict compilation guards
against nonstandard enum forward declarations.

The CMocka flags come from `pkg-config` by default. For local installations,
override `CMOCKA_CFLAGS` and `CMOCKA_LIBS`; `CPPCHECK` and `VALGRIND` can also
name custom executables. The sanitizer target defaults to leak detection. In
an environment where LeakSanitizer cannot run under process tracing, use
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`; Memcheck still checks leaks.

Cppcheck 2.13 incorrectly reports `unsignedLessThanZero` on the bounds checks
for the error message tables, so that diagnostic is suppressed. Other enabled
diagnostics fail the target.
