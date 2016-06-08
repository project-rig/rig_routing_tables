from rig.routing_table import MinimisationFailedError
from rig_routing_tables.utils import rig_to_c_table, c_to_rig_table
from _rig_routing_tables import lib


def minimise(table, target_length, no_raise=False):
    """Minimise a Rig routing table using the Ordered Covering algorithm [1]_.

    Parameters
    ----------
    routing_table : [:py:class:`~rig.routing_table.RoutingTableEntry`, ...]
        Routing table to minimise.
    target_length : int or None
        Target length of the routing table.

    Raises
    ------
    MinimisationFailedError
        If the smallest table that can be produced is larger than
        `target_length`.

    Returns
    -------
    [:py:class:`~rig.routing_table.RoutingTableEntry`, ...]
        Reduced routing table entries.

    [1]_ Mundy, A., Heathcote, J., Garside J.D., On-chip order-exploiting
    routing table minimization for a multicast supercomputer network. In *High
    Performance Switching and Routing (HPSR), 2016 International Conference on*
    IEEE (In Press)
    """
    # Convert the table to C format and minimise
    c_table = rig_to_c_table(table)
    lib.oc_minimise_na(c_table, target_length or 0)

    # If the table is larger than the target length then raise an exception
    if (not no_raise and target_length is not None and
            c_table.size > target_length):
        raise MinimisationFailedError(target_length, c_table.size)

    # Otherwise convert back to Rig form and return
    return c_to_rig_table(c_table)
