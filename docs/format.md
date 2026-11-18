# String formatting

`genesis_format` and `genesis_sformat` turn a format and arguments into a
string. The module does no file or terminal IO. Dynamic results are managed by
`lib/memory`: release them with `mem_free`. The fixed-buffer form returns the
full byte count excluding the terminating NUL, even when truncated. It
returns `SIZE_MAX` and sets `errno` on error.

The standard conversions are `d`, `i`, `o`, `u`, `x`, `X`, `f`, `F`, `e`, `E`,
`g`, `G`, `a`, `A`, `c`, `s`, `p`, `n`, and `%%`. Flags, width, precision, `*`, and
C length modifiers are parsed. Positional arguments are unsupported. Integer
and narrow text conversions are rendered by the module. Floating point,
pointer, and wide character/string conversions use `snprintf` for their
platform representation. A malformed format or unknown custom tag sets
`errno` to `EINVAL`.

Custom tags use `%{TAG}` or `%{TAG:options}`. Register with
`genesis_format_set` and remove with `genesis_format_rm`. Standard conversions
inside `options` are expanded before the callback, using arguments in their
usual left-to-right order. The tag then consumes one `(void *)value` argument.
Use `%%` to pass a literal percent. An invalid option format or failed option
expansion fails the whole formatting call without invoking that callback.
An option expansion follows the same output limit as a formatting call;
expansion side effects such as `%n` remain if a later conversion fails. An
option `%n` counts bytes produced by the option expansion itself.
A writer receives only `value` and the borrowed, expanded `options` string.
It returns a valid NUL-terminated string from
`mem_alloc`, `mem_realloc`, or another function returning memory managed by
`lib/memory`. The formatter takes ownership and calls `mem_free` after copying
it. On failure the writer returns `NULL` and sets `errno`.

The internal `COLOR` builtin consumes no value argument. Its options can still
contain standard conversions, which consume their own arguments. The public
`genesis_format_set` API continues to register callbacks that consume one
`(void *)` value.

```c
static char *as_text(const void *value, const char *options);

genesis_format_set("VALUE", as_text);
char *text = genesis_format("%{VALUE:short}", (void *)value);
/* use text */
mem_free(text);

/* The option argument precedes the custom value. */
text = genesis_format("%{VALUE:%s}", "short", (void *)value);
mem_free(text);
genesis_format_rm("VALUE");
```

The built-in `%{UUID}` tag accepts a borrowed `UUID *` value (including NULL,
which renders the nil UUID). Its default result is the canonical lower-case
36-character form. The options `upper` and `compact` can be combined with `;`
in either order: `upper` selects upper-case hex digits and `compact` removes
hyphens. Empty or unknown option segments fail with `errno = EINVAL`; an
entirely empty option string selects the default form.

The tag registry is an FNV-1a hash table with bucket chaining. Its internal
structures use the current Genesis-wide allocator from `core/setting`,
captured separately for each allocation. A global allocator change affects
future registry allocations directly. The
callback for an existing tag can be replaced without reallocating its storage.
The first registration creates the bucket table; removing the last tag releases
it. Both transitions are covered by regression tests.
Synchronize registry changes with concurrent formatting.

`genesis_format_settings` contains only format behavior choices. Currently,
`max_output` limits produced bytes excluding NUL. Its initial value is
`SIZE_MAX - 1`; zero permits only empty output. Edit the public workspace and
call its no-argument `apply()` function to publish a validated private
configuration. Direct edits have no runtime effect; failed apply calls
preserve the previous limit.

```c
genesis_format_settings.max_output = 1024;
genesis_format_settings.apply();
char *text = genesis_format("%s", "hello");
mem_free(text);
```

Formatting proceeds left to right. If a later conversion fails, earlier `%n`
stores and callback side effects remain. The dynamic result is released, or
the fixed buffer is emptied. A nonzero fixed buffer is NUL-terminated on
success and error.
