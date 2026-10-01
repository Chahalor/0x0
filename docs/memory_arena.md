# Memory arenas

`mem_arena_new()` creates an arena owned by the memory manager. Its initial
capacity is fixed at 64 KiB. `mem_arena_alloc()` returns zeroed storage aligned
for any standard C object. An allocation that would exceed the capacity returns
`NULL` with `errno = ENOMEM`; the arena does not grow. Arena allocations remain
valid until they are rewound, reset, or the arena is released. Do not pass an
individual arena allocation to `mem_free()`.

`mem_arena_mark()` records the current position. `mem_arena_rewind()` invalidates
only allocations made after that mark and preserves all earlier bytes. Marks
created after the target mark become invalid; the target mark can be reused.
`mem_arena_reset()` invalidates every allocation and mark but keeps the backing
storage. `mem_arena_trim()` releases backing storage only when the arena is
empty, so it never moves a live pointer. A later allocation restores the fixed
capacity.

`mem_arena_dup()` returns a copy inside the arena. A positive size copies that
many bytes from a readable source. Size `-1` uses the recorded size of a block
allocated through the regular memory manager; an unregistered source requires
an explicit size.

`mem_arena_free()` releases an arena. `mem_free_all()` releases every arena as
well as regular managed blocks. All arena allocations and releases use the
current `G_SETTINGS.allocator` callbacks directly. Release live arenas before
switching to incompatible callbacks. Synchronize arena use with concurrent
memory operations; arenas are not internally locked.
