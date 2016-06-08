"""Utilities for converting routing tables between different formats."""
from rig.routing_table import RoutingTableEntry, Routes
from _rig_routing_tables import ffi, lib
from six import iteritems
import struct


def rig_to_c_table(table):
    """Convert a Rig-format routing table into the format expected by the C
    functions.

    Parameters
    ----------
    table : [:py:class:`rig.routing_table.RoutingTableEntry`]
        List of routing table entries.

    Returns
    -------
    table
        C format routing table.
    """
    # Create the table
    c_table_raw = lib.new_table(len(table))
    assert c_table_raw != ffi.NULL
    c_table = ffi.gc(c_table_raw, lib.free_table)

    # Add the entries
    for i, entry in enumerate(table):
        # Get the relevant C entry
        c_entry = c_table.entries[i]

        # Set up as required
        c_entry.keymask.key = entry.key
        c_entry.keymask.mask = entry.mask

        # Compute the route
        c_entry.route = 0x0
        for r in entry.route:
            c_entry.route |= 1 << r

        # Compute the source, including the MSB if None is present in the set
        c_entry.source = 0x0
        for r in entry.sources:
            c_entry.source |= 1 << (r if r is not None else 31)

    return c_table


def c_to_rig_table(c_table):
    """Convert a C-format routing table into the format expected by Rig
    methods.

    Parameters
    ----------
    c_table :
        C format routing table (e.g., as constructed by
        :py:func:`rig_to_c_table`.

    Returns
    -------
    table : [:py:class:`rig.routing_table.RoutingTableEntry`]
        List of routing table entries.
    """
    # Construct the table by iterating over the entries in the C format
    table = list()
    for entry in (c_table.entries[i] for i in range(c_table.size)):
        # Get the source and route
        route = {r for r in Routes if (1 << r) & entry.route}
        source = {r for r in Routes if (1 << r) & entry.source}

        # Special bit indicating absence of knowledge about some sources
        if entry.source >> 31:
            source.add(None)

        # Add the new entry to the table
        table.append(RoutingTableEntry(route, entry.keymask.key,
                                       entry.keymask.mask, source))

    return table


def read_routing_tables(fp):
    """Read routing tables from a file.

    Parameters
    ----------
    fp : file object
        File object containing routing tables.

    Each routing table starts with two bytes representing the x and y
    co-ordinates of the chip to which the routing table relates (a byte for
    each of the co-ordinates) and a short (two bytes) representing the number
    of entries in the following table. A table entry consists of four words,
    one each for the key, the mask, the source and the route fields.

    Yields
    ------
    (x, y)
        Co-ordinates of the routing table.
    [:py:class:`rig.routing_table.RoutingTableEntry`, ...]
        Routing table found at this co-ordinate.
    """
    for xy, c_table in read_routing_tables_in_c_format(fp):
        yield xy, c_to_rig_table(c_table)


def write_routing_tables(tables, fp):
    """Write routing tables to a file.

    Parameters
    ----------
    tables : {(x, y): [:py:class:`rig.routing_table.RoutingTableEntry`, ...]}
        Dictionary mapping chip co-ordinates to the routing table associated
        with that chip.
    fp : file object
    """
    write_routing_tables_from_c_format(
        {chip: rig_to_c_table(table) for chip, table in iteritems(tables)}, fp
    )


def read_routing_tables_in_c_format(fp):
    """Read routing tables from a file.

    Parameters
    ----------
    fp : file object
        File object containing routing tables.

    Yields
    ------
    (x, y)
        Co-ordinates of the routing table.
    c_table
        Routing table found at this co-ordinate (in C format).
    """
    while True:
        # Try to read the header
        header = fp.read(4)
        if header == b'':
            break
        assert len(header) == 4, "Truncated header"

        x, y, length = struct.unpack("<2BH", header)

        # Create the routing table
        c_table_raw = lib.new_table(length)
        assert c_table_raw != ffi.NULL
        c_table = ffi.gc(c_table_raw, lib.free_table)

        # Read in the entries
        for entry in (c_table.entries[i] for i in range(length)):
            data = fp.read(16)
            assert len(data) == 16, "Truncated entry"
            (entry.keymask.key, entry.keymask.mask,
             entry.source, entry.route) = struct.unpack("<4I", data)

        yield (x, y), c_table


def write_routing_tables_from_c_format(tables, fp):
    """Write routing tables to a file.

    Parameters
    ----------
    tables : {(x, y): c_table}
        Dictionary mapping chip co-ordinates to the routing table associated
        with that chip.
    fp : file object
    """
    # Write each table in turn, buffering in memory to save writes
    for (x, y), table in iteritems(tables):
        # Create the buffer
        data = bytearray(4 + 16*table.size)

        # Write the header
        struct.pack_into("<2BH", data, 0, x, y, table.size)

        # Dump in the entries
        for i in range(table.size):
            entry = table.entries[i]
            struct.pack_into("<4I", data, 4 + 16*i,
                             entry.keymask.key,
                             entry.keymask.mask,
                             entry.source,
                             entry.route)

        # Write out
        fp.write(data)
