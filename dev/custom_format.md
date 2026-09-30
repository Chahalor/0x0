# Genesis - custom format

The goal of the format module is to handle the standard C format (%s, %d, %c, ...) and even more

## Custom tags

The format module will let the developper setup custom flags.
The function of the dev called by the module should return a dynamicly allocated and valid C string. This string must be managed by lib/memory so the formatter can release it with mem_free. The fucntion must loose any ownership over the returned string.

The format is:
- At first the user setup tag
- then separated by ':', option passed to the user funtion. This is not in the current version but the format parsing must handle this.
```c
"%{TAG:opt:opt:opt:...}"
```
example:
```c
printf("%s\n", genesis_format("%{JSON}", json_elt)); // <- should print the json strigifyed
```

