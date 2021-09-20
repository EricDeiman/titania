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


#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "codebuffer.hh"
#include "ir.hh"

int
main( int argc, char **argv ) {

    CodeBuffer cb{ "!" };
    std::ifstream is{ argv[ 1 ] };

    while( !is.eof() ) {
        char buffer[ 1024 ];
        is.getline( buffer, sizeof( buffer ) );
        cb.writeCodeBuffer( { buffer } );
    }

    std::vector< CodeBuffer >vcb;
    vcb.push_back( cb );

    IR ir{ std::move( vcb ) };
    // ir.mkBasicBlocks();
    //ir.testLocalValueNumbering( "!", "!", std::cout );
    // ir.dumpBasicBlocks( std::cout );

    // ir.testTreeHeightBalance( "!", "!", std::cout );

    return 0;
}