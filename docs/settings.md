# Genesis settings

`core/setting` provides one interface for module configuration. A caller owns a
`t_genesis_settings` object and passes it to modules that need configuration.
Managers are independent; changing one does not change another.

```c
#include "core/core.h"

t_genesis_settings settings;
t_genesis_core_allocator allocator;
t_genesis_core_buffer_settings growth;
t_genesis_core_buffer *buffer;

genesis_settings_init(&settings);
allocator = core_allocator_system();
/* Replace allocator callbacks and context here if needed. */
genesis_settings_set(&settings, &genesis_allocator_setting, &allocator);
growth = (t_genesis_core_buffer_settings){64, BUFFER_GROWTH_EXACT};
genesis_settings_set(&settings, &genesis_buffer_setting, &growth);
buffer = core_buffer_new_with_settings(64, &settings);
core_buffer_free(buffer);
genesis_settings_deinit(&settings);
```

`genesis_settings_set` validates and copies the complete module value.
`genesis_settings_get` copies the override, or asks the module for its default.
`genesis_settings_reset` removes an override. A key without an override or a
default produces `-SETTING_ERROR_NOT_CONFIGURED`. The functions return `0` on
success, positive standard errors, or negative module errors as defined in
[norm.md](norm.md).

Each module can expose a public `setting.h` with a stable
`t_genesis_setting_key` and a settings type from its `types.h`. The key gives
the value size and optional callbacks for defaults and validation. The core
manager identifies a key by its address, so the descriptor must outlive every
manager using it. Modules can add a new key without changing the manager or
adding a central module enum. The allocator and buffer keys in
`src/core/allocator/setting.h` and `src/core/buffer/setting.h` are examples.
The buffer key selects geometric or exact capacity growth.

The manager copies values into private storage. Its new entries use the
allocator currently selected by `genesis_allocator_setting`; each entry keeps
that allocator for its own deallocation. A buffer created with a manager also
copies its allocator and growth policy, so later settings changes do not alter
existing buffers.
The allocator context must remain valid until all allocations made with it are
released. Do not copy an initialized manager; call `genesis_settings_deinit`
when finished. Synchronize access if sharing one manager across threads.

The older `core_settings_default`, `core_settings_set_allocator`, and
`core_settings_allocator` helpers remain for existing callers. New module
configuration uses `genesis_settings_init/set/get/reset/deinit`.

`lib/memory` still selects its tracking structure at compile time through
`MEM_MANAGER_TYPE`. Runtime selection through this interface requires the
memory module to keep both implementations available and consume its own
settings key.
