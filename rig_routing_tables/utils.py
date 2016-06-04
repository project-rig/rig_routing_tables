"""Utilities for converting routing tables between different formats."""
from rig.routing_table import RoutingTableEntry, Routes
from _rig_routing_tables import ffi, lib


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
