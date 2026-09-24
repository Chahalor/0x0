# Genesis - Architecture

## File achitecture

### dev dir
The dev dir must contaigne any file dev related that is not documentation or approuve an valid script used by the build/developement process

### Docs dir
The docs dir contaigne all docs that approuve, about effective and current comportement of the lib

### mk dir
This dir contaign the templates for all makefile used accros the differents modules.

### scripts dir
This dir has all the script used and validated for the developement, build and packaging of the lib.

### src dir
This dir contaign all the source code of the lib, each sub directory is a differents "product" that are indenedent in they design one to an other. They can have dependences to each other (ex: lib depend on the core) but should not share code outside of a well docuemented and limited API.

### tests dir
The test dir contaigne all the tests for CI/CD purposes.

### Tools dir
This dir contaign usefull tools for the developement of the lib.

## Lib file architecture
The core and the lib follow the same file logic and separation, They are separated in defferente modules. A module is defined by a group of sub modules or a public API hidding the real implementation behind the internal part of the module

A sub module follow this kind of arch:
```
src/lib/files/json
├── _internal_
│   ├── _access.c
│   ├── _check.c
│   ├── edit
│   │   ├── _json_edit.h
│   │   ├── _set.c
│   │   ├── _set_logic.c
│   │   └── _unset.c
│   ├── _format.c
│   ├── _get.c
│   ├── _json.h
│   ├── _memory.c
│   ├── _parser.c
│   ├── _string.c
│   ├── _tools.c
│   ├── _types.h
│   └── _write.c
├── json.c
├── makefile
└── types.h
```

A public part with the public APi so others modules and devs can use it. This part should not leak any of the internal function, macro or types nor have any real implementation. The public functions should not have any logic, they are only allowed to check the params validity.
A internal part with all the module implementation and logic. none of the code from this part should leak to the others modules.
Each module should provide a makefile ready to be included inside the main makefile. Have a clear and documented error handling with publci errors codes and functions to use thems.
The diffentes modules should respect the files roles. The public API of each module should be in module_name.h, the type exported by this module in types.h and the public implementation of each API functions in module_name.c.

## Module settings

A configurable module exposes its settings type in `types.h` and a stable
`t_genesis_setting_key` in its public `setting.h`. The key provides the value
size and optional default and validation callbacks. Callers use the common
`genesis_settings_set/get/reset` interface with that key. Modules read their
configuration from a caller-owned `t_genesis_settings` object and copy any
values that must remain valid after the object changes or is released. A
module can add a new key without editing the core settings manager. See
[settings.md](settings.md) for the current API and ownership rules.

## Error convention

- A function returning a pointer returns `NULL` when it cannot provide a pointer. When applicable, it sets `errno` to a standard positive error code.
- A function returning an error code returns `0` on success, a positive standard `errno` value (`ENOMEM`, `EINVAL`, etc.) for a system error, or a negative module-specific error code.
- Each module defines its errors in an enum named for that module. Its first value is `MODULE_ERROR_SUCCESS = 0`; its last value is `MODULE_ERROR_UNKNOWN`. The enum values are nonnegative array indices. Return `-MODULE_ERROR_NAME` for a module-specific error.
- Each module exposes a `strerror` function that accepts its returned code. It describes `0`, positive standard errors, known negative module errors and unknown negative codes. Module messages are stored in a `const char *const` array using designated initializers such as `[MODULE_ERROR_NAME] = "message"`; the lookup uses `-code` as its index. Positive standard errors use the platform `strerror`. Error messages must not be selected by a chain of `if` or `switch` cases.
- `genesis_strerror(module, code)` dispatches through a constant array of module message functions indexed by `module`. The returned string is borrowed and must not be freed.

```c
enum e_genesis_core_buffer_error
{
	BUFFER_ERROR_SUCCESS = 0,
	BUFFER_ERROR_SOURCE_OUT_OF_RANGE = 1,
	BUFFER_ERROR_UNKNOWN = 2
};
```

```c
int status = core_buffer_append(&buffer, data, length);
if (status != 0)
	fprintf(stderr, "%s\n", genesis_strerror(GENESIS_MODULE_BUFFER, status));
```

## Code format
### File layout
Each file has a special and mandatory file layout based on it role in the API. and use the layout descibled in `/home/nduvoid/.config/Code/User/snippets`.
A types file only has types, struct, enums and unions declaration.
A interface header includes the types header and only has the API function and MACRO.
And a C file respect the layout by keeping separated the local and exported functions.

### Function format
A fucntion declaration/definition should follow this format:
```c
int	core_buffer_resize(
	t_genesis_core_buffer	*const	buffer,
	const size_t					new_length
);

int	core_buffer_useless_function(void);
```
A tab after the type, each param on it own line, alligning with tabs pointer and params names. A no_param function should have the void keyword.

Every `return` statement that returns a value must enclose the complete value or expression in parentheses: `return (value);`. This also applies to constants, pointers and function calls. A `void` function still uses `return;` when needed.

Inside of the fucntion implementation, variables must be declared at the begining of they scope. The allignement rules for pointer and name also apply here and if the variable is assigned the differents equal must be alligned with tabs.

After a scope end (while, if, for, ...) a line break must be performed.

```c
	const unsigned char	*source				= data;
	size_t				source_offset		= 0;
	int					source_is_internal	= 0;
	size_t				new_length;
	int					result;

	if (!buffer_is_valid(buffer) || (!data && length != 0))
		return (EINVAL);

	if (length > SIZE_MAX - buffer->length)
		return (EOVERFLOW);

	if (length == 0)
		return (0);

	new_length = buffer->length + length;
	if (new_length > buffer->capacity && buffer->bytes)
	{
		uintptr_t	base	= (uintptr_t)buffer->bytes;
		uintptr_t	address	= (uintptr_t)data;

		if (address >= base && address - base < buffer->capacity)
		{
			source_offset = (size_t)(address - base);
			if (source_offset > buffer->length
				|| length > buffer->length - source_offset)
				return (-BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
			source_is_internal = 1;
		}
	}
```

The allignement rule also apply to the structs, unions, enums and typedefs.
structs name must begin with `s_`.
unions name must begin with `u_`.
enums name must begin with `e_`.
typedefs name must begin with `t_`.
```c
typedef struct s_genesis_core_buffer	t_genesis_core_buffer;

struct s_genesis_core_buffer
{
	unsigned char				*bytes;
	size_t						length;
	size_t						capacity;
	t_genesis_core_allocator	allocator;
};

enum e_genesis_core_buffer_enum
{
	CORE_BUFFER_OK				= 0,
	CORE_BUFFER_NO_MEM			= 0,
	CORE_BUFFER_INVALID_FILE	= 0,
	CORE_BUFFER_UNKNOW			= 0
};
```

## Documentation
### in code docs

The docs for any part of the code must follow the doxygen format.
Each header file must have A header before the protection macro with is goal, scope and modules.
Each fucntion of a public API must document they params, returns values, actions and have at least one usage example. Private one must be more implemetation oriented.
