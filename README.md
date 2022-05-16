# Simple Buffer String (SBS)

A reformulation of [SDS](https://github.com/antirez/sds) to use string
buffers... no heap allocations.

## Motivation

Have an SDS API for short term bounded strings without heap allocation.
Frequently, I've want to build strings up where it is reasonable to expect they
won't grow larger than a certain size. When using SDS it was a frequent
painpoint to have to handle freeing the strings when also performing error
checking.

## Differences from SDS

- No heap allocation
- String must be accessed via `sbsstr()`
- sbs struct is mutated instead of returning a new struct

## SDS API Mapping

A quick reference for those familiar with the SDS API:

| SDS               | SBS                           | Additional Info                                               |
| ----------------- | ----------------------------- | ------------------------------------------------------------- |
| `sdsnewlen`       | `sbsnewlen`/`SBSNEWLEN`       | -                                                             |
| `sdsnew`          | `sbsnew`/`SBSNEW`             | -                                                             |
| `sdsempty`        | `sbsempty`/`SBSEMPTY`         | -                                                             |
| `sdsdup`          | `sbsdup`/`SBSDUP`             | -                                                             |
| `sdslen`          | `sbslen`                      | -                                                             |
| `sdsfree`         | ❌                            | No resource management                                        |
| `sdscatlen`       | `sbscatlen`                   | -                                                             |
| `sdscat`          | `sbscat`                      | -                                                             |
| `sdscatsds`       | `sbscatsbs`                   | -                                                             |
| `sdsgrowzero`     | `sbsresize`/`SBSRESIZE`       | Provides a new buffer & the extra buffer space is not zero'ed |
| `sdscatprintf`    | `sbscatprintf`                | -                                                             |
| `sdscatvprintf`   | `sbscatvprintf`               | -                                                             |
| `sdscatfmt`       | `sbscatfmt`                   | -                                                             |
| `sdsfromlonglong` | `sbsfromlonglong`/`SBSFROMLL` | -                                                             |
| `sdstrim`         | `sbstrim`                     | -                                                             |
| `sdsrange`        | `sbsrange`                    | -                                                             |
| `sdscpylen`       | `sbscpylen`                   | -                                                             |
| `sdscpy`          | `sbscpy`                      | -                                                             |
| `sdscatrepr`      | `sbscatrepr`                  | -                                                             |
| `sdssplitlen`     | ❌                            | May later be added via an iterator?                           |
| `sdsfreesplitres` | ❌                            | No resource management                                        |
| `sdssplitargs`    | ❌                            | May later be added via an iterator?                           |
| `sdsjoin`         | `sbsjoin`                     | -                                                             |
| `sdsjoinsds`      | `sbsjoinsbs`                  | -                                                             |

## Error Recovery

SBS is designed to revert the string to previous state if the operation fails.
The underlying buffer may be modified during a failing operation, but the string
itself will stay the same. This is important as it is much more likely for an
SBS operation to fail than SDS due to the buffer size vs the likelihood of a
failed malloc.

## Creation

It is recommended that for creating new SBS strings that you use the built-in
macros. These macros not only reduce boilerplate code, but also ensure that a
buffer is not accidentally shared between multiple sbs structs. These macros
rely on compound literals to allow the creation of anonymous structs on the
stack.

The preferred way for strings is using `SBSNEW()` or one of the preset sizes
`SBS64()`/`SBS128()`/`SBS256()`/`SBS512()`/`SBS1024()`/`SBS2048()`.

✅ Best Way

```c
sbs* text1 = SBSNEW("so short to write", 64); // the size must be a literal
sbs* text2 = SBS64("even shorter");
sbs* text3 = SBS2048("bigger buffer");

char raw_data[] = {65, 65, 0x0, 0x2};
sbs* data = SBSNEWLEN(raw_data, sizeof(raw_data), 64);
```

⛔️ Bad Way

```c
// exposes the internal buffer, making it easy to accidentally reuse
char buffer[64];
sbs text; // all functions prefer the pointer as the value
sbsnew(&text, "so much boilerplate", buffer, sizeof(buffer));
```

The creation functions all return either `sbs*` or `NULL` if there is a failure.
This is perhaps the only place a segfault may occur if you do not do a `NULL`
check in scenarios where initialization will fail.

## Properties

Although the structure is transparent, it is best practice to prefer the
property functions.

- `sbsstr()`: returns the c string/bytes
- `sbslen()`: returns the length of the c string/bytes
- `sbssize()`: returns the size of the buffer
- `sbsavail()`: returns the amount of space left in the buffer.
  [Escape hatch](#escape-hatch) property.
- `sdsend()`: returns the position of the end of the string/bytes in the buffer.
  [Escape hatch](#escape-hatch) property.

## Escape Hatch

Sometimes the functionality available in the SBS API is not sufficient. As such
there are a few functions available that allow you to modify the sbs string and
then fix the sbs struct accordingly.

## Development

Even though I am not using this in an embedded environment. I hope this library
can also be useful for such users as well. PRs are welcome to help address any
portability/embedability issues of the library.

- c99 is soft-required for the convenience macros due to compound-literals
