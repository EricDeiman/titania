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

#ifndef CFG_HH
#define CFG_HH

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

#include "basicblock.hh"
#include "codebuffer.hh"

// One control flow graph per function
class Cfg {
public:
    Cfg( CodeBuffer & );

    vector< BasicBlock > basicBlocks;

    int
    getOffset( string name );

    string
    getName();

private:
    void
    mkBasicBlock( CodeBuffer & );

    bool
    isJumpInsr( string );

    void
    addBlock( BasicBlock &&block ) {
        auto name{ block.name };
        auto offset{ basicBlocks.size() };

        basicBlocks.push_back( move( block ) );
        blockNames[ name ] = offset;
    }

    unordered_map< string, size_t > blockNames;

    string name;

};

#endif