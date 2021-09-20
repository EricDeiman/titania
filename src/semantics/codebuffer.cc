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

#include "codebuffer.hh"
#include <string>

CodeBuffer::CodeBuffer( std::string id, bool global ) : name( id ) {

    // since zero is false, it will be handy to have in a register
    auto freg = getFreshRegister();
    auto treg = getFreshRegister();

    if( global ) {
        codeBuffer.push_back( "!:" );
        codeBuffer.push_back( "loadi 0 => " + freg );
        codeBuffer.push_back( "loadi 1 => " + treg );
    }

    valuesScopes.push_back( valuesMap );

    valuesScopes.back()[ "0" ] = freg;
    valuesScopes.back()[ "false" ] = freg;
    valuesScopes.back()[ "1" ] = treg;
    valuesScopes.back()[ "true" ] = treg;
}

std::string
CodeBuffer::getName() {
    return name;
}

std::string
CodeBuffer::getFreshRegister() {
    return "r" + std::to_string( registerNum++ );
}

std::string
CodeBuffer::getFreshCCRegister() {
    return "cc" + std::to_string( ccNum++ );
}

std::string
CodeBuffer::makeLabel( std::string base ) {
    return base + std::to_string( labelSuffix++ );
}

std::vector< std::string >
CodeBuffer::makeLabel( std::vector< std::string > prefixes ) {
    std::vector< std::string >labels;

    for( auto p : prefixes ) {
        labels.push_back( p + std::to_string( labelSuffix ) );
    }

    labelSuffix++;

    return std::move( labels );
}

void
CodeBuffer::writeCodeBuffer( std::vector< std::string > data ) {

    std::string buffer;
    for( auto s : data ) {
        buffer += s;
    }

    codeBuffer.push_back( buffer );
}

size_t
CodeBuffer::valuesScopesCount( std::string id ) {

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            return m.count( id );
        }
    }

    return 0;
}

std::string
CodeBuffer::valuesScopesLookup( std::string id ) {

    std::string tmp;

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            tmp = m[ id ];
        }
    }

    return tmp;
}

std::ostream&
CodeBuffer::dumpCodeBuffer( std::ostream &os ) {
    for( auto s : codeBuffer ) {
        os << s << std::endl;
    }

    os << std::endl;

    return os;
}

std::vector< std::string >*
CodeBuffer::getBuffer() {
    return &codeBuffer;
}

std::vector< BasicBlock >&
CodeBuffer::getBlocks() {
    return basicBlocks;
}