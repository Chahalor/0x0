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

The CMocka contract suite covers the current `core` modules and the reviewed
`lib` modules: memory, random, times, log, format and UUID.

Install CMocka development headers and library, Valgrind, and Cppcheck. A C11
compiler with AddressSanitizer and UndefinedBehaviorSanitizer support is also
required. Then run:

```sh
make test
```

`make test` delegates to `tests/Makefile` and runs these checks in order:

| Target | Check |
| --- | --- |
| `test-strict` | Build core and reviewed lib sources with C11, pedantic warnings, and `-Werror`; run contract tests. |
| `test-sanitize` | Run the same tests with AddressSanitizer and UndefinedBehaviorSanitizer. |
| `test-valgrind` | Run the strict binaries under Memcheck, failing on errors or leaks. |
| `test-cppcheck` | Analyze core, format, memory and UUID sources with Cppcheck warnings, style, performance and portability checks. |

`make -C tests test-core` runs only the core groups. To run one contract group,
use `make -C tests test-settings`, `test-buffer`, `test-errors`,
`test-memory`, `test-memory-bucket`, `test-memory-arena`,
`test-memory-arena-bucket`, `test-utilities`, `test-log`, `test-format` or
`test-uuid` with the same `make -C tests` prefix. Archive integrity is checked
with `make -C tests test-archive`. The root `make norm` runs Cppcheck through
`tests/Makefile`.

`make -C tests test-dev-format` is an optional local regression check for the
ignored `.dev/main.c` demo. It builds the archive and checks the COLOR, UUID,
and JSON argument order through the demo's command-line output.

The tests exercise normal results, invalid arguments, error codes, global
allocator callbacks, buffer growth and boundary behavior. Regression cases
cover buffer self-append during reallocation, zero-capacity buffers, failed
initialization, direct memory use of global allocator callbacks,
and first custom tag registration. Strict compilation guards against
nonstandard enum forward declarations.

The CMocka flags come from `pkg-config` by default. For local installations,
override `CMOCKA_CFLAGS` and `CMOCKA_LIBS`; `CPPCHECK` and `VALGRIND` can also
name custom executables. The sanitizer target defaults to leak detection. In
an environment where LeakSanitizer cannot run under process tracing, use
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`; Memcheck still checks leaks.

Cppcheck 2.13 incorrectly reports `unsignedLessThanZero` on the bounds checks
for the error message tables, so that diagnostic is suppressed. Other enabled
diagnostics fail the target.
