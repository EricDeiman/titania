#ifndef iloci_hh
#define iloci_hh

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class State {
public:
    State( size_t regCount = 256, size_t memSize = 1024 ) : registers( regCount ), 
                                                            ccregs( regCount ),
                                                            memory( memSize ) {}

    std::vector< std::int64_t >
    registers;

    std::vector< std::int64_t >
    memory;

    std::vector< int >
    ccregs;

    std::unordered_map< std::string, size_t >
    labelOffsets;

    std::int64_t
    tos = 0;

    std::int64_t
    arp = 0;

    bool
    running = true;

    size_t
    insrPtr = 0;
};

#endif