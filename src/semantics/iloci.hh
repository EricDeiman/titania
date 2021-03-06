/*
  The Titania programming language
  Copyright 2021 Eric J. Deiman

  This file is part of the Titania programming language.
  The Titania programming language is free software: you can redistribute it
  and/ormodify it under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  The Titania programming language is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with the
  Titania programming language. If not, see <https://www.gnu.org/licenses/>
*/

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

    std::vector< std::vector< std::int64_t > >
    registersStack;

    std::vector< std::int64_t >
    memory;

    std::vector< int >
    ccregs;

    std::vector< std::vector< int > >
    ccregsStack;

    std::unordered_map< std::string, size_t >
    labelOffsets;

    std::int64_t
    tos = -1;

    std::int64_t
    arp = 0;

    bool
    running = true;

    size_t
    insrPtr = 0;
};

#endif