from rig.routing_table import RoutingTableEntry as RTE
from rig.routing_table import Routes
from rig_routing_tables import utils


def test_table_to_cffi_and_back():
    """Test conversion of a Rig routing table to the CFFI format and conversion
    back again.
    """
    # Create the initial table
    table = [RTE({r}, i, 0xffffffff) for i, r in enumerate(Routes)]
    table += [
        RTE({Routes.north, Routes.south}, 0x80000000, 0xffffffff,
            {Routes.east, Routes.west}),
    ]

    # Convert the table to C format
    c_table = utils.rig_to_c_table(table)

    # Check for equivalence
    assert c_table.size == len(table)
    for i, entry in enumerate(table):
        c_entry = c_table.entries[i]

        assert c_entry.keymask.key == entry.key
        assert c_entry.keymask.mask == entry.mask

        # Check the route is equivalent
        route = 0x0
        for r in entry.route:
            route |= 1 << r

        assert c_entry.route == route

        # Check the source is equivalent
        source = 0x0
        for r in entry.sources:
            source |= 1 << (r if r is not None else 31)

        assert c_entry.source == source

    # Convert back again
    returned_table = utils.c_to_rig_table(c_table)
    assert table == returned_table
