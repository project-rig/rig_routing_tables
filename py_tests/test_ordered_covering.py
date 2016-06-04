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
