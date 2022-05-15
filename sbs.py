import ctypes as ct
from typing import Any, Union

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


libsbs.sbscatlen.errcheck = errcheck


def sbscatlen(s: sbs, text: bytes) -> sbs:
    libsbs.sbscatlen(ct.byref(s), text, ct.c_size_t(len(text)))
    return s


libsbs.sbscat.errcheck = errcheck


def sbscat(s: sbs, text: str) -> sbs:
    libsbs.sbscat(ct.byref(s), text.encode())
    return s


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
