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
                                                            memory( memSize ) {}

    std::vector< size_t >
    registers;

    std::vector< size_t >
    memory;

    std::unordered_map< std::string, size_t >
    labelOffsets;

    size_t
    tos = 0;

    size_t
    arp = 0;

    bool
    running = true;

    size_t
    insrPtr = 0;
};

#endif