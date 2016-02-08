"""Use a SpiNNaker implementation of Ordered Covering to minimise routing
tables.
"""
from rig.machine_control import MachineController
from rig.routing_table import RoutingTableEntry, Routes
import struct


def get_memory_profile(mc):
    """Return the cumulative heap usage over time."""
    # Keep a track of how much memory is associated with each pointer,
    # track cumulative memory usage over time
    usage = [0]
    pointers = dict()

    # Read the linked list of entry arrays
    buf = mc.read_vcpu_struct_field("user0")
    while buf != 0x0:
        # Unpack the header
        n_entries, buf_next = struct.unpack("<2I", mc.read(buf, 8))
        buf += 8

        # Read in the memory recording entries
        for _ in range(n_entries):
            n_bytes, ptr = struct.unpack("<2I", mc.read(buf, 8))
            buf += 8

            if n_bytes == 0:
                # This is a free
                usage.append(usage[-1] - pointers.pop(ptr))
            else:
                # This is an allocation
                usage.append(usage[-1] + n_bytes)
                pointers[ptr] = n_bytes

        # Progress to the next block of memory
        buf = buf_next

    return usage


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
    table = [None for __ in range(length)]
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
    mc = MachineController("192.168.1.1")
    mc.send_signal("stop")

    # Write the table table into memory on chip (0, 0)
    print("Loading tables...")
    with mc(x=0, y=0):
        mem = mc.sdram_alloc_as_filelike(len(table)*12 + 8, tag=1)
        mem.write(pack_table(table, 0))

    # Load the application
    print("Loading app...")
    mc.load_application("./ordered_covering_profiled.aplx", {(0, 0): {1}})

    # Wait until this does something interesting
    print("Minimising...")
    ready = mc.wait_for_cores_to_reach_state("exit", 1, timeout=5.0)
    if ready < 1:
        raise Exception("Something didn't work...")

    # Read back the table
    print("Reading back table...")
    mem.seek(0)
    new_table = unpack_table(mem.read())

    print("\n---")
    for entry in new_table:
        print("{!s}".format(entry))
    print("---\n")

    # Read back the memory profile
    print("Reading back memory profile...")
    with mc(x=0, y=0, p=1):
        get_memory_profile(mc)
    mc.send_signal("stop")
