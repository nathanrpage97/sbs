import ctypes as ct
from typing import Union

libsbs = ct.CDLL("libsbs.so")

class sbs(ct.Structure):
    size: int
    _fields_ = [
        ("_str", ct.c_void_p),
        ("_len", ct.c_size_t),
        ("size", ct.c_size_t)
    ]

    @property
    def str(self) -> str:
        return self.bytes.decode()

    @property
    def bytes(self) -> bytes:
        return ct.cast(self._str, ct.POINTER(ct.c_char * self._len)).contents.value


class SBSException(Exception):
    pass

# text = Union[str, bytes]

def errcheck(result: int, *args) -> int:
    if result != 0:
        raise SBSException()
    return result


libsbs.sbsnewlen.restype = sbs
def sbsnewlen(text: bytes, *, size: int) -> sbs:
    buffer = ct.create_string_buffer(size)
    val = libsbs.sbsnewlen(text, ct.c_size_t(len(text)), ct.byref(buffer), ct.c_size_t(size))
    val._buffer = buffer # store to prevent garbage collection
    return val

libsbs.sbsnew.restype = sbs
def sbsnew(text: str, *, size: int) -> sbs:
    buffer = ct.create_string_buffer(size)
    val = libsbs.sbsnewlen(text.encode(), ct.c_size_t(len(text)), ct.byref(buffer), ct.c_size_t(size))
    val._buffer = buffer # store to prevent garbage collection
    return val

libsbs.sbscatlen.errcheck = errcheck
def sbscatlen(s: sbs, text: bytes) -> sbs:
    libsbs.sbscatlen(ct.byref(s), text, ct.c_size_t(len(text)))
    return s

libsbs.sbscat.errcheck = errcheck
def sbscat(s: sbs, text: str) -> sbs:
    libsbs.sbscat(ct.byref(s), text.encode())
    return s

