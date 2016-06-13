import pytest
from rig.routing_table import RoutingTableEntry as RTE
from rig.routing_table import Routes, MinimisationFailedError
from rig_routing_tables.ordered_covering import minimise


@pytest.mark.parametrize("target_length, no_raise",
                         ((None, False), (0, True)))
def test_ordered_covering_simple(target_length, no_raise):
    """Test that a very simple routing table can be minimised, only one
    merge should be made as there are no conflicts.

    Table::

        0000 -> N, NE
        0001 -> N, NE
        001X -> S

    Can be minimised to::

        000X -> N, NE
        001X -> S
    """
    # Original table
    table = [
        RTE({Routes.north, Routes.north_east}, 0b0000, 0b1111),
        RTE({Routes.north, Routes.north_east}, 0b0001, 0b1111),
        RTE({Routes.south}, 0b0010, 0b1110),
    ]

    # Expected table
    expected_table = [
        RTE({Routes.north, Routes.north_east}, 0b0000, 0b1110),
        RTE({Routes.south}, 0b0010, 0b1110),
    ]

    # Minimise and check the result
    new_table = minimise(table, target_length, no_raise)

    assert new_table == expected_table


def test_ordered_covering_simple_fails_if_too_large():
    """Test that a very simple routing table can be minimised, and that an
    exception is raised if that minimisation is still too large.

    Table::

        0000 -> N, NE
        0001 -> N, NE
        001X -> S
    """
    # Original table
    table = [
        RTE({Routes.north, Routes.north_east}, 0b0000, 0b1111),
        RTE({Routes.north, Routes.north_east}, 0b0001, 0b1111),
        RTE({Routes.south}, 0b0010, 0b1110),
    ]

    with pytest.raises(MinimisationFailedError) as exc:
        minimise(table, target_length=1)

    assert exc.value.target_length == 1
    assert exc.value.final_length == 2
    assert exc.value.chip is None


def test_sort_table_before_minimisation():
    """Test that the routing table is reordered before insertion.

    The following table won't minimise, but should be reordered:

        01XX -> N
        001X -> NE
        0000 -> E
    """
    table = [
        RTE({Routes.north}, 0x4, 0xc),
        RTE({Routes.north_east}, 0x2, 0xe),
        RTE({Routes.east}, 0x0, 0xf),
    ]

    # Minimise (should re-order)
    minimised = minimise(list(table), None)

    # Assert the reordering occurred
    assert minimised == table[-1::-1]
