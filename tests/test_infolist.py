import pytest

from obit import InfoList


@pytest.fixture
def info_list():
    return InfoList.InfoList()


def test_get_nonexistent(info_list):
    res = info_list.get("name")

    assert res[0] != 0


def test_set_int(info_list):
    info_list.set("int", 1)
    assert info_list.get("int") == [0, "int", 4, [1, 1, 1, 1, 1], [1]]

    info_list.set("long", 2, "long")
    assert info_list.get("long") == [0, "long", 4, [1, 1, 1, 1, 1], [2]]


def test_set_float(info_list):
    info_list.set("float", 1.234)
    res = info_list.get("float")
    assert res[0] == 0
    assert res[1] == "float"  # name
    assert res[2] == 10  # OBIT_float == 10
    assert res[4] == pytest.approx([1.234])

    info_list.set("double", 1.234, "double")
    res = info_list.get("double")
    assert res[0] == 0
    assert res[1] == "double"  # name
    assert res[2] == 11  # OBIT_double == 11
    assert res[4] == pytest.approx([1.234])


def test_set_str(info_list):
    info_list.set("str", "qwerty")

    assert info_list.get("str") == [0, "str", 14, [6, 1, 1, 1, 1], ["qwerty"]]


def test_pget(info_list):
    # Check exception on non-existent value
    with pytest.raises(KeyError):
        InfoList.PGet(info_list, "a")

    info_list.set("a", 111)

    assert InfoList.PGet(info_list, "a") == [0, "a", 4, [1, 1, 1, 1, 1], [111]]


def test_always_put_int(info_list):
    value_int = 123
    dim = [1, 1, 1, 1, 1]

    InfoList.PAlwaysPutInt(info_list, "val", dim, [value_int])
    assert info_list.get("val") == [0, "val", 4, dim, [value_int]]


def test_always_put_float(info_list):
    value_float = 1.234
    dim = [1, 1, 1, 1, 1]

    InfoList.PAlwaysPutFloat(info_list, "val", dim, [value_float])
    assert info_list.get("val") == [0, "val", 10, dim, pytest.approx([value_float])]

    InfoList.PAlwaysPutDouble(info_list, "val", dim, [value_float])
    assert info_list.get("val") == [0, "val", 11, dim, pytest.approx([value_float])]


def test_always_put_str(info_list):
    value_str = "raes03ad"
    dim = [8, 1, 1, 1, 1]

    InfoList.PAlwaysPutString(info_list, "val", dim, [value_str])
    assert info_list.get("val") == [0, "val", 14, dim, [value_str]]
