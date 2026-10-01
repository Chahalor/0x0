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
`genesis_format_set` and remove with `genesis_format_rm`. Each tag consumes one
argument passed as `(void *)value`. A writer receives only `value` and a
borrowed `options` string. It returns a valid NUL-terminated string from
`mem_alloc`, `mem_realloc`, or another function returning memory managed by
`lib/memory`. The formatter takes ownership and calls `mem_free` after copying
it. On failure the writer returns `NULL` and sets `errno`.

```c
static char *as_text(const void *value, const char *options);

genesis_format_set("VALUE", as_text);
char *text = genesis_format("%{VALUE:short}", (void *)value);
/* use text */
mem_free(text);
genesis_format_rm("VALUE");
```

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
