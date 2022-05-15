# Simple Buffer String (SBS)

A reformulation of [SDS](https://github.com/antirez/sds) to use string buffers.

## Motivation

Have an SDS API for short term bounded strings without heap allocation.

## Differences from SDS

- No malloc/free
- String must be accessed via `sbsstr()`
- sbs struct is mutated instead of returning a new object

## SDS API Mapping

A quick reference for those familiar with the SDS API:

| SDS               | SBS                     | Additional Info                                               |
| ----------------- | ----------------------- | ------------------------------------------------------------- |
| `sdsnewlen`       | `sbsnewlen`/`SBSNEWLEN` | -                                                             |
| `sdsnew`          | `sbsnew`/`SBSNEW`       | -                                                             |
| `sdsempty`        | `sbsempty`/`SBSEMPTY`   | -                                                             |
| `sdsdup`          | `sbsdup`/`SBSDUP`       | -                                                             |
| `sdslen`          | `sbslen`                | -                                                             |
| `sdsfree`         | ❌                      | No resource management                                        |
| `sdscatlen`       | `sbscatlen`             | -                                                             |
| `sdscat`          | `sbscat`                | -                                                             |
| `sdscatsds`       | `sbscatsbs`             | -                                                             |
| `sdsgrowzero`     | `sbsresize`/`SBSRESIZE` | Provides a new buffer & the extra buffer space is not zero'ed |
| `sdscatprintf`    | `sbscatprintf`          | -                                                             |
| `sdscatvprintf`   | `sbscatvprintf`         | -                                                             |
| `sdscatfmt`       | `sbscatfmt`             | -                                                             |
| `sdsfromlonglong` | `sbsfromlonglong`       | Must be on an initialized sbs                                 |
| `sdstrim`         | `sbstrim`               | -                                                             |
| `sdsrange`        | `sbsrange`              | -                                                             |
| `sdscpylen`       | `sbscpylen`             | -                                                             |
| `sdscpy`          | `sbscpy`                | -                                                             |
| `sdscatrepr`      | `sbscatrepr`            | -                                                             |
| `sdssplitlen`     | ❌                      | May later be added via an iterator?                           |
| `sdsfreesplitres` | ❌                      | No resource management                                        |
| `sdssplitargs`    | ❌                      | May later be added via an iterator?                           |
| `sdsjoin`         | `sbsjoin`               | -                                                             |
| `sdsjoinsds`      | `sbsjoinsbs`            | -                                                             |

## Error handling

SBS is designed to leave the string as is if the operation fails. The underlying
buffer may be modified, but the string itself will stay the same. This is
important as it is much more likely for an SBS operation to fail than SDS due to
the bounded buffer size vs a failed malloc.

## Non-Goals

- Shared buffer for multiple strings. Use the SBS creation macros to help keep
  the buffers separate.
- (Currently) these functions may not be the most performant as possible (PRs
  welcome).
