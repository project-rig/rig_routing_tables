"""Use an application to load routing tables for us.
"""
from rig.machine_control import MachineController
from rig.routing_table import RoutingTableEntry, Routes
import struct


def pack_table(table, app_id):
    """Pack a routing table into the form required for dumping into SDRAM."""
    data = bytearray(3*4 + len(table)*4*4)

    # Pack the header
    struct.pack_into("<3I", data, 0, app_id, 0x0, len(table))

    # Pack in the entries
    offset = 3*4
    for entry in table:
        pack_rte_into(entry, data, offset)
        offset += 16

    return data


def pack_rte_into(rte, buf, offset):
    """Pack a routing table entry into a buffer."""
    # Construct the source integer
    source = 0x0
    for r in rte.sources:
        if r is not None:
            source |= 1 << r

    # Construct the route integer
    route = 0x0
    for r in rte.route:
        route |= 1 << r

    # Pack
    struct.pack_into("<4I", buf, offset, rte.key, rte.mask, route, source)


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
    with mc.application(57):
        # Write the table table into memory on chip (0, 0)
        print("Loading tables...")
        with mc(x=0, y=0):
            mem = mc.sdram_alloc_as_filelike(len(table)*16 + 12, tag=1)
            mem.write(pack_table(table, 57))

        # Load the application
        print("Loading app...")
        mc.load_application("./rt_minimise.aplx", {(0, 0): {1}})

        # Wait until this does something interesting
        print("Minimising...")
        ready = mc.wait_for_cores_to_reach_state("exit", 1, timeout=5.0)
        if ready < 1:
            print(mc.get_iobuf(x=0, y=0, p=1))
            raise Exception("Something didn't work...")

    print("\nReading minimised table...")
    for e in mc.get_routing_table_entries(x=0, y=0):
        if e is not None:
            entry, app, _ = e
            print("{:d}: {:#010x}/{:#010x} --> {}".format(
                app, entry.key, entry.mask, entry.route)
            )

    print("\nStopping application...")
    mc.send_signal("stop", app_id=57)

    for e in mc.get_routing_table_entries(x=0, y=0):
        if e is not None:
            entry, app, _ = e
            print("{:d}: {:#010x}/{:#010x} --> {}".format(
                app, entry.key, entry.mask, entry.route)
            )
