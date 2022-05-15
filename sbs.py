import ctypes as ct
from typing import Any, Sequence, Union

libsbs = ct.CDLL("libsbs.so")


class sbs(ct.Structure):
    size: int
    len: int
    _fields_ = [
        ("_str", ct.c_void_p),
        ("len", ct.c_size_t),
        ("size", ct.c_size_t),
    ]

    @property
    def str(self) -> str:
        return self.bytes.decode()

    @property
    def bytes(self) -> bytes:
        return ct.cast(self._str, ct.POINTER(ct.c_char * self.len)).contents.raw


class SBSException(Exception):
    pass


def errcheck(result: int, *args) -> int:
    if result != 0:
        raise SBSException()
    return result


libsbs.sbsnewlen.errcheck = errcheck


def sbsnewlen(text: bytes, *, size: int) -> sbs:
    buffer = ct.create_string_buffer(size)
    val = sbs()
    libsbs.sbsnewlen(
        ct.byref(val), text, ct.c_size_t(len(text)), ct.byref(buffer), ct.c_size_t(size)
    )
    val._buffer = buffer  # store to prevent garbage collection
    return val


libsbs.sbsnew.errcheck = errcheck


def sbsnew(text: str, *, size: int) -> sbs:
    buffer = ct.create_string_buffer(size)
    val = sbs()
    libsbs.sbsnewlen(
        ct.byref(val),
        text.encode(),
        ct.c_size_t(len(text)),
        ct.byref(buffer),
        ct.c_size_t(size),
    )
    val._buffer = buffer  # store to prevent garbage collection
    return val


libsbs.sbsempty.restype = sbs


def sbsempty(*, size: int) -> sbs:
    buffer = ct.create_string_buffer(size)
    val = libsbs.sbsempty(ct.byref(buffer), ct.c_size_t(size))
    val._buffer = buffer
    return val


libsbs.sbsdup.errcheck = errcheck


def sbsdup(s: sbs, d: sbs) -> sbs:
    libsbs.sbsdup(ct.byref(s), ct.byref(d))
    return d


libsbs.sbscatfmt.errcheck = errcheck


# something is not working here (c code is fine)


def sbscatfmt(s: sbs, format: str, *args: Any) -> sbs:
    libsbs.sbscatfmt(
        ct.byref(s),
        format.encode(),
        ct.c_int(32),
        ct.c_int(23),
    )
    return s


libsbs.sbscatprintf.errcheck = errcheck

# something is not working here (c code is fine)
def sbscatprintf(s: sbs, format: str, *args: Any) -> sbs:
    libsbs.sbscatprintf(
        ct.byref(s),
        format.encode(),
        *args,
    )
    return s


libsbs.sbsresize.errcheck = errcheck


def sbsresize(s: sbs, *, size: int) -> sbs:
    buf = ct.create_string_buffer(size)
    libsbs.sbsresize(ct.byref(s), ct.byref(buf), ct.c_size_t(size))
    s._buffer = buf  # prevent gc
    return s


libsbs.sbscatlen.errcheck = errcheck


def sbscatlen(s: sbs, text: bytes) -> sbs:
    libsbs.sbscatlen(ct.byref(s), text, ct.c_size_t(len(text)))
    return s


libsbs.sbscat.errcheck = errcheck


def sbscat(s: sbs, text: str) -> sbs:
    libsbs.sbscat(ct.byref(s), text.encode())
    return s


libsbs.sbscatsbs.errcheck = errcheck


def sbscatsbs(s: sbs, t: sbs) -> sbs:
    libsbs.sbscatsbs(ct.byref(s), ct.byref(t))
    return s


libsbs.sbscpylen.errcheck = errcheck


def sbscpylen(s: sbs, text: bytes) -> sbs:
    libsbs.sbscpylen(ct.byref(s), text, ct.c_size_t(len(text)))
    return s


libsbs.sbscpy.errcheck = errcheck


def sbscpy(s: sbs, text: str) -> sbs:
    libsbs.sbscpy(ct.byref(s), text)
    return s


libsbs.sbstrim.restype = None


def sbstrim(s: sbs, cset: str) -> sbs:
    libsbs.sbstrim(ct.byref(s), cset.encode())
    return s


libsbs.sbsrange.restype = None


def sbsrange(s: sbs, start: int, end: int) -> sbs:
    libsbs.sbsrange(ct.byref(s), ct.c_size_t(start), ct.c_size_t(end))
    return s


libsbs.sbsupdatelen.restype = None


def sbsupdatelen(s: sbs) -> sbs:
    libsbs.sbsupdatelen(ct.byref(s))
    return s


libsbs.sbsclear.restype = None


def sbsclear(s: sbs) -> sbs:
    libsbs.sbsclear(ct.byref(s))
    return s


def sbscmp(s1: sbs, s2: sbs) -> int:
    return libsbs.sbsclear(ct.byref(s1), ct.byref(s2))


libsbs.sbstolower.restype = None


def sbstolower(s: sbs) -> sbs:
    libsbs.sbstolower(ct.byref(s))
    return s


libsbs.sbstoupper.restype = None


def sbstoupper(s: sbs) -> sbs:
    libsbs.sbstoupper(ct.byref(s))
    return s


libsbs.sbsfromlonglong.errcheck = errcheck


def sbsfromlonglong(s: sbs, value: int) -> sbs:
    libsbs.sbsfromlonglong(ct.byref(s), ct.c_longlong(value))
    return s


libsbs.sbsjoin.errcheck = errcheck


def sbsjoin(s: sbs, args: Sequence[str], *, sep: str) -> sbs:
    argc = len(args)
    argv = (ct.c_char_p * argc)(*(arg.encode() for arg in args))
    libsbs.sbsjoin(
        ct.byref(s),
        ct.byref(argv),
        ct.c_int(argc),
        sep.encode(),
    )
    return s


libsbs.sbsjoinsbs.errcheck = errcheck


def sbsjoinsbs(s: sbs, args: Sequence[sbs], *, sep: str) -> sbs:
    argc = len(args)
    argv = (sbs * argc)(*args)
    libsbs.sbsjoinsbs(
        ct.byref(s), ct.byref(argv), ct.c_int(argc), sep.encode(), ct.c_size_t(len(sep))
    )
    return s
