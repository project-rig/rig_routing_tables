import pytest
from rig.routing_table import RoutingTableEntry as RTE
from rig.routing_table import Routes, MinimisationFailedError
from rig_routing_tables.mtrie import minimise


def test_minimise_simple():
    """Test minimisation of a simple routing table, we assume the C tests are
    extensive!
    """
    # Minimise the table:
    #    N  -> 0000 -> S SW
    #    NE -> 0001 -> S SW
    #    ?  -> 001X -> N
    table = [
        RTE({Routes.south, Routes.south_west}, 0x0, 0xf, {Routes.north}),
        RTE({Routes.south, Routes.south_west}, 0x1, 0xf, {Routes.north_east}),
        RTE({Routes.north}, 0x2, 0xe),
    ]

    # We expect the result
    #    N NE -> 000X -> S SW
    #    ?    -> 001X -> N
    expected = [
        RTE({Routes.south, Routes.south_west}, 0x0, 0xe,
            {Routes.north, Routes.north_east}),
        RTE({Routes.north}, 0x2, 0xe),
    ]

    # Try the minimisation
    assert minimise(table, None) == expected


def test_minimise_fails_if_oversize():
    """Test minimisation of a simple routing table, we assume the C tests are
    extensive!
    """
    # Minimise the table:
    #    N  -> 0000 -> S SW
    #    NE -> 0001 -> S SW
    #    ?  -> 001X -> N
    table = [
        RTE({Routes.south, Routes.south_west}, 0x0, 0xf, {Routes.north}),
        RTE({Routes.south, Routes.south_west}, 0x1, 0xf, {Routes.north_east}),
        RTE({Routes.north}, 0x2, 0xe),
    ]

    # We expect the result to be:
    #    N NE -> 000X -> S SW
    #    ?    -> 001X -> N
    # Which is, clearly, more than 1 entry so an error should be raised

    with pytest.raises(MinimisationFailedError) as exc:
        minimise(table, 1)

    assert exc.value.target_length == 1
    assert exc.value.final_length == 2
    assert exc.value.chip is None
