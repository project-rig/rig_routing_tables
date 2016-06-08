import pytest
from rig.routing_table import RoutingTableEntry as RTE
from rig.routing_table import Routes
from rig_routing_tables import utils
import struct
import tempfile


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


def test_read_routing_table_from_file():
    """Test reading a routing table from file."""
    # Write two routing tables to file in the appropriate format, then read
    # back and ensure the result is correct.
    fp = tempfile.TemporaryFile()

    # Write the first routing table:
    # (1, 2):
    #     S -> 00000000000000000000000000000000 -> NE 3
    #     E -> 00000000000000000000000000000001 -> S
    fp.write(struct.pack("<2BH", 1, 2, 2))
    fp.write(struct.pack("<4I", 0x0, 0xffffffff, 0b100000, 0b1000000010))
    fp.write(struct.pack("<4I", 0x1, 0xffffffff, 0b000001, 0b0000100000))

    # Write the second routing table:
    # (0, 0):
    #     ? -> 00000000000000000000000000000000 -> 1
    fp.write(struct.pack("<2BH", 0, 0, 1))
    fp.write(struct.pack("<4I", 0x0, 0xffffffff, 1 << 31, 0b10000000))

    # Check that the table is expected
    fp.seek(0)
    assert dict(utils.read_routing_tables(fp)) == {
        (1, 2): [
            RTE({Routes.north_east, Routes.core(3)}, 0x0, 0xffffffff,
                {Routes.south}),
            RTE({Routes.south}, 0x1, 0xffffffff, {Routes.east}),
        ],
        (0, 0): [
            RTE({Routes.core(1)}, 0x0, 0xffffffff),
        ],
    }

    fp.close()


def test_read_routing_table_from_file_truncated_header():
    fp = tempfile.TemporaryFile()

    # Write the first routing table:
    # (1, 2):
    #     S -> 00000000000000000000000000000000 -> NE 3
    #     E -> 00000000000000000000000000000001 -> S
    fp.write(struct.pack("<2BH", 1, 2, 2))
    fp.write(struct.pack("<4I", 0x0, 0xffffffff, 0b100000, 0b1000000010))
    fp.write(struct.pack("<4I", 0x1, 0xffffffff, 0b000001, 0b0000100000))

    # Write a truncated header
    fp.write(struct.pack("<2B", 0, 0))

    # Check that an exception is raised
    fp.seek(0)
    with pytest.raises(AssertionError):
        dict(utils.read_routing_tables(fp))

    fp.close()


def test_read_routing_table_from_file_truncated_entry():
    fp = tempfile.TemporaryFile()

    # Write the first routing table:
    # (1, 2):
    #     S -> 00000000000000000000000000000000 -> NE 3
    #     E -> 00000000000000000000000000000001 -> S
    fp.write(struct.pack("<2BH", 1, 2, 2))
    fp.write(struct.pack("<4I", 0x0, 0xffffffff, 0b100000, 0b1000000010))
    fp.write(struct.pack("<4I", 0x1, 0xffffffff, 0b000001, 0b0000100000))

    # Write the second routing table (truncated)
    # (0, 0):
    #     ? -> 00000000000000000000000000000000 -> 1
    fp.write(struct.pack("<2BHI", 0, 0, 1, 0))

    # Check that an exception is raised
    fp.seek(0)
    with pytest.raises(AssertionError):
        dict(utils.read_routing_tables(fp))

    fp.close()


def test_write_routing_tables():
    """Write to file and read back to check the table is correct."""
    tables = {
        (0, 0): [
            RTE({Routes.south, Routes.south_west}, 0x0, 0xf, {Routes.north}),
            RTE({Routes.south, Routes.south}, 0x1, 0xf, {Routes.north_east}),
            RTE({Routes.north}, 0x2, 0xe),
        ],
        (0, 1): [
            RTE({Routes.north}, 0x2, 0xe),
        ]
    }

    # Check on writing and reading
    fp = tempfile.TemporaryFile()
    utils.write_routing_tables(tables, fp)
    fp.seek(0)
    assert dict(utils.read_routing_tables(fp)) == tables

    fp.close()
