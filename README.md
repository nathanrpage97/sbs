# Simple Buffer String (SBS)

A reformulation of [SDS](https://github.com/antirez/sds) to use string buffers.

## Motivation

Have an SDS API for short term bounded strings without having to handle freeing.

## SDS API Mapping

A quick reference for those familiar with the SDS API:

| SDS               | SBS               | Additional Info                                                          |
| ----------------- | ----------------- | ------------------------------------------------------------------------ |
| `sdsnewlen`       | `sbsnewlen`       | -                                                                        |
| `sdsnew`          | `sbsnew`          | -                                                                        |
| `sdsempty`        | `sbsempty`        | -                                                                        |
| `sdsdup`          | `sbscpysbs`       | User must give an already created sbs. Closer to `memcpy` than `strdup`. |
| `sdslen`          | `sbslen`          | -                                                                        |
| `sdsfree`         | ❌                | No resource management                                                   |
| `sdscatlen`       | `sbscatlen`       | -                                                                        |
| `sdscat`          | `sbscat`          | -                                                                        |
| `sdscatsds`       | `sbscatsbs`       | -                                                                        |
| `sdsgrowzero`     | `sbsresize`       | User provides a new buffer to & the extra buffer space is not zero'ed    |
| `sdscatprintf`    | `sbscatprintf`    | -                                                                        |
| `sdscatvprintf`   | `sbscatvprintf`   | -                                                                        |
| `sdscatfmt`       | `sbscatfmt`       | -                                                                        |
| `sdsfromlonglong` | `sbsfromlonglong` | Must be on an initialized                                                |
| `sdstrim`         | `sbstrim`         | -                                                                        |
| `sdsrange`        | `sbsrange`        | -                                                                        |
| `sdscpylen`       | `sbscpylen`       | -                                                                        |
| `sdscpy`          | `sbscpy`          | -                                                                        |
| `sdscatrepr`      | `sbscatrepr`      | -                                                                        |
| `sdssplitlen`     | ❌                | May later be added via an iterator?                                      |
| `sdsfreesplitres` | ❌                | No resource management                                                   |
| `sdssplitargs`    | ❌                | May later be added via an iterator?                                      |
| `sdsjoin`         | `sbsjoin`         | -                                                                        |
| `sdsjoinsds`      | `sbsjoinsbs`      | -                                                                        |

## Error handling

SBS is designed to leave the string as is if the operation fails. The underlying
buffer may be modified, but the string itself will stay the same. This is
important as it is much more likely for an SBS operation to fail than SDS due to
the bounded buffer size vs a failed malloc.
