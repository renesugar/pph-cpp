pph
=========

pph generates a minimal order-preserving hash function for a list of keys.

Reference:

Practical perfect hashing
GV Cormack, RNS Horspool, M Kaiserswerth - The Computer Journal, 1985

# Table of Contents

- [License](#license)
- [Building](#building)
- [Using](#using)

# License

This project is licensed under the [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0).

# Building

This library uses the Boost library.

    export LDFLAGS="$LDFLAGS -L/path/to/boost/lib"
    export CPPFLAGS="$CPPFLAGS -I/path/to/boost/include"

pph uses CMake for its build system.

    mkdir build
    cd build
    cmake ..
    make

# Using

The basic command line to generate a hash function from a file containing a list of strings (one per line) is:

    pph -i ./file.txt -o ./file.hash

The command line to verify an existing hash function is:

    pph --verify ./file.hash

The other command line options can be seen by typing:

    pph --help

The default timeout for creating a hash function is 60000 milliseconds (1 minute).

If a hash function is not generated, you can try sorting the input file:  

    pph -i file.txt --index > file_index.txt
    sort --numeric-sort --key=2 file_index.txt > file_sorted_index.txt
    awk -F' ' '{print $1}'  file_sorted_index.txt > file_sorted.txt


# Python

This library uses the Boost library. Install the Boost library and set **LDFLAGS** and **CPPFLAGS** before installing the Python module.

    export LDFLAGS="$LDFLAGS -L/path/to/boost/lib"
    export CPPFLAGS="$CPPFLAGS -I/path/to/boost/include"
    
Install the module.

    pip3 install pph

Import the module.

    from pph import PphHashTable, PphRandomNumber, PphKeyFunctions

See the tests for how to generate a hash function using the Python interface.    
