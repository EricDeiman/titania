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

using namespace std;

#include "codebuffer.hh"
#include "ir.hh"

int
main( int argc, char **argv ) {

    CodeBuffer cb{ "!" };
    ifstream is{ argv[ 1 ] };

    while( !is.eof() ) {
        char buffer[ 1024 ];
        is.getline( buffer, sizeof( buffer ) );
        cb.writeCodeBuffer( { buffer } );
    }

    vector< CodeBuffer >vcb;
    vcb.push_back( cb );

    IR ir{ move( vcb ), argv[ 1 ] };
    
    ir.doSuperLocalValueNumbering();

    string fileName{ argv[ 1 ] };
    auto baseFileName{ fileName.substr( 0, fileName.find_last_of( "." ) ) };
    ofstream outFile{ baseFileName + ".slvn.iloc" };
    ir.dumpCfgBasicBlocks( outFile );

    // ir.testTreeHeightBalance( "!", "!", cout );

    return 0;
}