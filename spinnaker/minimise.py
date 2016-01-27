"""Use a SpiNNaker implementation of Ordered Covering to minimise routing
tables.
"""
from rig.machine_control import MachineController
from rig.routing_table import RoutingTableEntry, Routes
import struct


def pack_table(table, target_length):
    """Pack a routing table into the form required for dumping into SDRAM."""
    data = bytearray(2*4 + len(table)*3*4)

    # Pack the header
    struct.pack_into("<2I", data, 0, len(table), target_length)

    # Pack in the entries
    offset = 8
    for entry in table:
        pack_rte_into(entry, data, offset)
        offset += 12

    return data


def pack_rte_into(rte, buf, offset):
    """Pack a routing table entry into a buffer."""
    # Construct the route integer
    route = 0x0
    for r in rte.route:
        route |= 1 << r

    # Pack
    struct.pack_into("<3I", buf, offset, rte.key, rte.mask, route)


def unpack_table(data):
    # Unpack the header
    length, _ = struct.unpack_from("<2I", data)

    # Unpack the table
    table = [None for _ in range(length)]
    for i in range(length):
        key, mask, route = struct.unpack_from("<3I", data, i*12 + 8)
        routes = {r for r in Routes if (1 << r) & route}
        table[i] = RoutingTableEntry(routes, key, mask)

    return table


if __name__ == "__main__":
    # Construct the test table
    RTE = RoutingTableEntry
    table = [
        RTE({Routes.north, Routes.north_east}, 0b0000, 0b1111),
        RTE({Routes.east}, 0b0001, 0b1111),
        RTE({Routes.south_west}, 0b0101, 0b1111),
        RTE({Routes.north, Routes.north_east}, 0b1000, 0b1111),
        RTE({Routes.east}, 0b1001, 0b1111),
        RTE({Routes.south_west}, 0b1110, 0b1111),
        RTE({Routes.north, Routes.north_east}, 0b1100, 0b1111),
        RTE({Routes.south, Routes.south_west}, 0b0100, 0b1111),
    ]

    # Talk to the machine
    mc = MachineController("192.168.240.253")

    # Write the table table into memory on chip (0, 0)
    with mc(x=0, y=0):
        mem = mc.sdram_alloc_as_filelike(len(table)*12 + 8)
        mem.write(pack_table(table), 0)

    # Load the application
    mc.load_application("./ordered_covering.aplc", {(0, 0): {1}})

    # Wait until this does something interesting
    ready = mc.wait_for_cores_to_reach_state("exit", 1, timeout=5.0)
    if ready < 1:
        raise Exception("Something didn't work...")

    # Read back the table
    mem.seek(0)
    new_table = unpack_table(mem.read())

    print(new_table)
