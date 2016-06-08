from setuptools import setup, find_packages

with open("rig_routing_tables/version.py", "r") as f:
    exec(f.read())

setup(
    name="rig_routing_tables",
    version=__version__,
    packages=find_packages(),

    # Files required by CFFI wrapper
    package_data={
        'rig_routing_tables': ['../include/*.h',
                               'rig_routing_tables/cffi_utils.c',
                               'rig_routing_tables/cffi_utils.h']
    },

    # Metadata for PyPi
    url="https://github.com/project-rig/rig_routing_tables",
    author="The Rig Authors",
    description="A C library (and CFFI Python Interface) for "
                "routing table minimisation.",
    license="GPLv2",
    classifiers=[
        "Development Status :: 3 - Alpha",

        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",

        "License :: OSI Approved :: GNU General Public License v2 (GPLv2)",

        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",

        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",

        "Topic :: Software Development :: Libraries",
    ],
    keywords="spinnaker cffi routing-table-minimization",

    # Build CFFI Interface
    cffi_modules=["rig_routing_tables/cffi_compile.py:ffi"],
    setup_requires=["cffi>=1.0.0"],
    install_requires=["cffi>=1.0.0", "rig>=1.0.0, <2.0.0"],
)
