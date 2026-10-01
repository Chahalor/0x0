# UUID

Include `types/uuid/uuid.h` for the public API and
`types/uuid/setting.h` to configure generation. The older `types/uuid.h`
include remains an alias.

The module generates version 7 UUIDs using the current time and a random
source. The default source is the system `getrandom` function. To replace it,
assign a callback to `genesis_uuid_setting.getrandom` and call
`genesis_uuid_setting.apply()`. The callback has the same signature as
`getrandom`; short reads are retried. The default version is 7, and other
versions are currently rejected. A failed `apply()` leaves the last applied
configuration active. Synchronize setting changes with concurrent generation.

`genesis_uuid()`, `genesis_uuid_generate(7)`, and `genesis_uuid_read()` return
owned UUID values. Release them with `genesis_uuid_free()`. The string from
`genesis_uuid_stringify()` is a tracked allocation; release it with
`mem_free()`. The setter and parser functions accept caller-owned UUID values
and preserve them when an operation fails. Formatting a `NULL` UUID yields the
nil UUID text.

Functions returning an integer follow the Genesis error convention: zero for
success, positive `errno` values for system errors, and negative UUID error
values for module errors. `genesis_uuid_strerror()` describes these values;
`genesis_uuid_error_module` works with `genesis_strerror()`. Pointer-returning
functions report failure with `NULL` and set `errno`.
