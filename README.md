# Routing Table Tools

[![Build Status](https://travis-ci.org/project-rig/rig_routing_tables.svg?branch=docs)](https://travis-ci.org/project-rig/rig_routing_tables)
[![Coverage](https://codecov.io/github/project-rig/rig_routing_tables/coverage.svg?branch=master)](https://codecov.io/github/project-rig/rig_routing_tables?branch=master)

`rig_routing_tables` contains C implementations of routing table minimisation
algorithms for the SpiNNaker platform.

## Building SpiNNaker executables

To build SpiNNaker executables (`*.aplx`) ensure you have the [latest version
of SpiNNaker
tools](http://apt.cs.manchester.ac.uk/projects/SpiNNaker/downloads/) installed.
Then switch to the `spinnaker` directory and call `make`.

Further documentation on the SpiNNaker executables is in the `spinnaker`
directory.

## Building desktop executables

Switch to the `desktop` directory and call `make`.

Further documentation on the desktop executables is in the `desktop` directory.

## Building and running tests

Documentation on building and running tests is present in the `tests`
directory. Tests are built using the [Check](http://libcheck.github.io/check/)
framework.
