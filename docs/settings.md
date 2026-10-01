# Genesis settings

`core/setting` holds the allocator callbacks shared by Genesis. The public
`G_SETTINGS` alias refers to `genesis_global_settings`. Its allocator starts
with `malloc`, `free`, and `realloc`. Applications can replace the three
callbacks at runtime, without recompiling Genesis:

```c
#include "core/setting/setting.h"

G_SETTINGS.allocator.alloc = custom_alloc;
G_SETTINGS.allocator.free = custom_free;
G_SETTINGS.allocator.realloc = custom_realloc;
```

Changes take effect directly for allocations and releases. Set all callbacks
before using a module and synchronize changes with concurrent operations.
`memory` reads `G_SETTINGS.allocator` for every allocation and release; it
does not store a private copy. Release live memory before switching to
incompatible callbacks.
The `format` tag registry records the release callback for each tag and bucket
array, so previously registered tags remain removable after a callback change.
Core buffers use the current global callbacks for growth and release; release
active buffers before changing the allocator.

Module settings are separate. For example, `format` exposes a public
`genesis_format_settings` workspace for its output limit. Edit it and call
its no-argument `apply()` to publish the behavior choice. The module uses the
last successfully applied limit:

```c
genesis_format_settings.max_output = 1024;
genesis_format_settings.apply();
```

Strings returned by `genesis_format` belong to `memory` and are released with
`mem_free`. Custom writers return `memory` managed strings too. Module
settings do not select an allocator.
