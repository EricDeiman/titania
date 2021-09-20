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

// Encapulate the state needed for function and hold the code generated for it.
#ifndef codebuffer_hh
#define codebuffer_hh

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "basicblock.hh"

class CodeBuffer {

public:
    CodeBuffer() = default;

    CodeBuffer( std::string, bool = false );

    CodeBuffer &
    operator=( const CodeBuffer & ) = default;
    
    void
    writeCodeBuffer( std::vector< std::string > );

    std::string
    getFreshRegister();

    // CC stands for condition code
    std::string
    getFreshCCRegister();

    std::string
    makeLabel( std::string );

    std::vector< std::string >
    makeLabel( std::vector< std::string > );

    size_t
    valuesScopesCount( std::string );

    std::string
    valuesScopesLookup( std::string );

    std::unordered_map< std::string, std::string > 
    valuesMap;

    std::vector< std::unordered_map< std::string, std::string > >
    valuesScopes;

    std::ostream&
    dumpCodeBuffer( std::ostream &os );

    std::string
    getName();

    std::vector< std::string >*
    getBuffer();

    std::vector< BasicBlock >&
    getBlocks();

    std::vector< BasicBlock >basicBlocks;

private:
    std::vector< std::string > codeBuffer;

    int registerNum = 0;
    int ccNum = 0;
    int labelSuffix = 0;

    std::string name;
};

#endif