import sbs
import pytest
import ctypes as ct


def test_sbsnew() -> None:
    sbs.sbsnew("test", size=12)


def test_sbscatlen() -> None:
    text = sbs.sbsnew("test", size=18)
    text = sbs.sbscatlen(text, b"test\x00cont")
    # assert text.bytes == b"testtest\x00cont"
    assert text.len == len(b"testtest\x00cont")
    with pytest.raises(sbs.SBSException):
        sbs.sbscatlen(text, b"really long text that overflows")


def test_sbscat() -> None:
    text = sbs.sbsnew("test", size=12)
    text = sbs.sbscat(text, "test")
    assert text.str == "testtest"
    assert text.size == 12

    with pytest.raises(sbs.SBSException):
        sbs.sbscat(text, "really long text that overflows")

    with pytest.raises(sbs.SBSException):
        text = sbs.sbscat(text, "1234")  # check the exact boundary

    text = sbs.sbscat(text, "123")
    assert text.str == "testtest123"


# def test_sbscatfmt() -> None:
#     text = sbs.sbsempty(size=24)
#     val1, val2 = ct.c_int(23), ct.c_int(32)
#     text = sbs.sbscatfmt(text, "%i %i", val1, val2)

#     assert text.str == "23 32"


# def test_sbscatprintf() -> None:
#     text = sbs.sbsempty(size=24)
#     val1, val2 = ct.c_int(23), ct.c_int(32)
#     text = sbs.sbscatprintf(text, "%d %d", val1, val2)
#     val1, val2
#     assert text.str == "23 32"


def test_sbsdup() -> None:
    text = sbs.sbsempty(size=24)
    text = sbs.sbscat(text, "this is some text")

    with pytest.raises(sbs.SBSException):
        sbs.sbsdup(text, size=2)
