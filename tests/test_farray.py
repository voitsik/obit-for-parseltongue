import pytest

from obit import FArray


def test_create_1d():
    arr = FArray.FArray("myarr", [5])

    assert repr(arr) == "<C FArray instance> myarr"

    assert arr.Count == 5
    assert arr.Ndim == 1
    assert arr.Naxis == [5]


def test_create_2d():
    arr = FArray.FArray("myarr", [30, 50])

    assert repr(arr) == "<C FArray instance> myarr"

    assert arr.Count == 30 * 50
    assert arr.Ndim == 2
    assert arr.Naxis == [30, 50]


def test_get_set():
    arr = FArray.FArray("arr", [10])

    arr.set(1.234, 5)

    assert arr.get(5) == pytest.approx(1.234)
