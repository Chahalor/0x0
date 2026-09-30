# Genesis settings

`core/setting` owns values shared across Genesis, including the default
allocator. A developer edits the global settings workspace and calls its
no-argument `apply()` function. A zero allocator descriptor selects the
system allocator (`malloc`, `realloc`, and `free`). A successful apply changes
the value read by future operations; allocations that already exist keep the
allocator needed for their release.

```c
#include "genesis.h"

genesis_global_settings.allocator = custom_allocator;
genesis_global_settings.apply();
```

Each module owns only settings for its algorithms and behavior. A module
exposes a public settings struct with an `apply` function pointer and a named
no-argument apply function. Edits to the public struct have no effect until
apply validates and publishes a complete private configuration. A failed
apply leaves the previous configuration active. Ordinary module calls take no
settings parameter.

For example, `format` currently exposes only its output limit:

```c
#include "interface/format/setting.h"

genesis_format_settings.max_output = 1024;
genesis_format_settings.apply();
```

Ordinary dynamic values use `lib/memory`. Release strings returned by
`genesis_format` with `mem_free`; custom format writers return strings managed
by `lib/memory` as well. Internal opaque structures may allocate directly
through the applied global allocator and retain the matching release callback
when the global setting changes.

The older caller-owned `t_genesis_settings` manager and the core APIs that use
it remain as compatibility code for existing core consumers. Their migration
is outside the current `core/setting` and `format` update. This legacy manager
is not the configuration model for new modules.
