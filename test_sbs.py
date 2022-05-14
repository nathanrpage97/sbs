import sbs
import pytest


def test_sbsnew() -> None:
    sbs.sbsnew("test", size=12)


def test_sbscatlen() -> None:
    text = sbs.sbsnew("test", size=18)
    text = sbs.sbscatlen(text, b"test\x00cont")
    # assert(text.bytes == b"testtest\x00cont")

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
