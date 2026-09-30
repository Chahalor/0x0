# Genesis Settings Model

```
Public settings
    Mutable freely by the application.
    Never consumed directly by the module.

apply()
    Validate.
    Normalize.
    Build a complete runtime configuration.
    Atomically/safely publish it.

Internal configuration
    Only source of truth used by the module.
```

## Goal

Provide a simple and consistent configuration model for Genesis modules without exposing runtime state directly or requiring a large setter API.

## Responsibility of `core/setting`

`core/setting` owns only values shared across Genesis, such as the default
allocator. It does not own module-specific settings or publish module runtime
configurations. A module reads shared global values when needed for internal work. Its own
settings contain only algorithm and behavior choices.

For the allocator, the public `genesis_global_settings` workspace has its own
no-argument `apply()` pointer. `genesis_default_allocator()` returns the last
applied value. A zero allocator descriptor selects the system allocator.

Each module owns its public settings workspace, its `apply()` function, and its
private applied configuration. Changing a shared global default affects future operations that read it.
Existing internal allocations keep the allocator needed for their release.

The current caller-owned `t_genesis_settings` manager in `core/setting` and
the APIs that depend on it are legacy code to migrate. The migration must
preserve existing users until their replacement APIs are available.

## Model

### Public settings

Each module exposes a public settings structure.

```c
typedef struct s_genesis_x_settings
{
    /* user-configurable fields */
    int (*apply)(void);
} t_genesis_x_settings;

extern t_genesis_x_settings genesis_x_settings;
```

Rules:

- Freely mutable by the application.
- Used only as a configuration workspace.
- Never read directly by the module runtime.
- Changes have no effect until `apply()` is called.
- `apply` points to the public no-argument apply function.

### `apply()`

`apply()` is the only bridge between public settings and the module runtime configuration.

```c
int genesis_x_settings_apply(void);

/* Equivalent calls: */
genesis_x_settings.apply();
genesis_x_settings_apply();
```

It must:

1. Validate the public settings.
2. Normalize values.
3. Build a complete internal runtime configuration.
4. Publish the new configuration safely / atomically.

If validation or publication fails, the current runtime configuration must remain valid and unchanged.

### Internal configuration

Each module maintains a private runtime configuration.

```c
typedef struct s_genesis_x_config
{
    /* resolved runtime values */
} t_genesis_x_config;
```

Rules:

- Private to the module.
- Only source of truth used by runtime code.
- Never modified directly by the application.
- Never partially updated.
- Replaced only through the settings application process.

## Data flow

```text
Public settings
      |
      | genesis_x_settings_apply()
      v
Validate
Normalize
Build runtime config
      |
      v
Safely publish
      |
      v
Internal configuration
      |
      v
Module runtime
```

## Core invariant

> Runtime code must never consume public settings directly.

This separation allows the public API to stay simple while keeping validation, consistency, thread-safety, and future implementation changes under the control of each module.

## Open points

To refine later:

- Whether settings can be reapplied after module initialization.
	- it depend on the module internal configuration. This must be definable inside the module internal part
- Runtime reconfiguration strategy:
	- safe publication of new configurations;
	- lifetime/reclamation of replaced configurations.
- Whether some settings are startup-only while others are runtime-mutable.
	- all must be untime-mutable, but some settings could also be unmutable, it depend on the module configuration
- Standard error codes returned by `apply()`.
	- internal system error	-> return the related errno
	- already applyed		-> setting can be applyed one time and has already been applyed
	- invalid setting value	-> cal also just be EINVAL
