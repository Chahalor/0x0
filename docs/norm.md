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

A configurable module exposes in its public `setting.h` a mutable settings
struct, a global instance, and a no-argument apply function. The struct holds
an `apply` function pointer to that function. Applications edit the public
instance and call apply to validate, normalize, and publish
a complete private runtime configuration. Runtime module code reads only the
last successfully applied private configuration. Direct public edits have no
effect, and failed apply calls leave that configuration unchanged.
Ordinary module operations need no settings argument. Module settings contain
only algorithm and behavior choices. Ordinary allocated values use lib/memory
and are released with mem_free; internal opaque structures may capture the
global allocator for their own release. Existing per-instance
APIs may retain caller-owned `t_genesis_settings` objects. See
[settings.md](settings.md) for details.

`core/setting` is responsible only for global values shared across Genesis,
such as the default allocator. Module-specific public settings, validation,
application, and private runtime configuration belong to each module. A module
reads shared values from core/setting when it needs them; module apply only
publishes module behavior choices.
The existing caller-owned settings manager remains a compatibility interface
until its consumers are migrated.

## Error convention

- A function returning a pointer returns `NULL` when it cannot provide a pointer. When applicable, it sets `errno` to a standard positive error code.
- A function returning an error code returns `0` on success, a positive standard `errno` value (`ENOMEM`, `EINVAL`, etc.) for a system error, or a negative module-specific error code.
- Each module defines its errors in an enum named for that module. Its first value is `MODULE_ERROR_SUCCESS = 0`; its last value is `MODULE_ERROR_UNKNOWN`. The enum values are nonnegative array indices. Return `-MODULE_ERROR_NAME` for a module-specific error.
- Each module exposes a `strerror` function that accepts its returned code. It describes `0`, positive standard errors, known negative module errors and unknown negative codes. Module messages are stored in a `const char *const` array using designated initializers such as `[MODULE_ERROR_NAME] = "message"`; the lookup uses `-code` as its index. Positive standard errors use the platform `strerror`. Error messages must not be selected by a chain of `if` or `switch` cases.
- Each module exports a constant `t_genesis_error_module` descriptor containing its name and `strerror` function. `genesis_strerror(&module_descriptor, code)` dispatches through that descriptor, so a new module does not require a core enum or dispatcher edit. The returned string is borrowed and must not be freed.

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
	fprintf(stderr, "%s\n", genesis_strerror(&genesis_buffer_error_module, status));
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

## Tests

For every new module or public API change, define the module before writing its
implementation. Complete these steps in order:

1. Define the public API in the module's public headers: functions, exported
   types, settings and error codes.
2. Document each function's contract: valid and invalid inputs, return value
   and error codes, output and state changes, ownership and lifetime, defaults,
   and boundary behavior where applicable.
3. Write CMocka tests from those contracts. Include normal, invalid and
   boundary cases before implementing the function.
4. Connect the module's tests to the Make targets. The public headers,
   contracts, test suite and build skeleton form the module's initial template.
5. Only then implement the module and make the contract tests pass.

An API change updates its contract and tests before its implementation. A bug
fix first adds a test that reproduces the bug, then changes the implementation.
The regression test remains in the suite. Existing modules can adopt this
sequence as their public APIs are reviewed.

Public module APIs are tested with CMocka. Each public contract needs a test
for return values, error codes, defaults, boundary behavior and ownership when
applicable. Every corrected behavior bug gets a regression test that fails
before the fix and remains in the suite.

`make test` runs strict compilation, AddressSanitizer and
UndefinedBehaviorSanitizer, Valgrind Memcheck and Cppcheck. Each check also has
an individual Make target. See [testing.md](testing.md) for the current suite
and commands.
