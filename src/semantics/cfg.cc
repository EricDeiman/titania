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


#include <algorithm>
#include <string>

using namespace std;

#include "cfg.hh"

Cfg::Cfg( CodeBuffer &fnCode ) {
    name = fnCode.getName();
    mkBasicBlock( fnCode );
}

int
Cfg::getOffset( string name ) {
    if( 0 < blockNames.count( name ) ) {
        return blockNames[ name ];
    }

    return -1;
}

string
Cfg::getName() {
    return name;
}

void
Cfg::mkBasicBlock( CodeBuffer &code ) {
    auto buffer{ code.getBuffer() };
    BasicBlock bb;
    bool blockStarted = false;

    for( auto i = 0; i < buffer->size(); i++ ) {
        if( buffer->at( i ).find_first_of( ':' ) != string::npos ) {
            if( blockStarted ) {
                // finish the existing block
                bb.endIdx = i - 1;
                auto label{ buffer->at( i ) };
                auto id{ label.substr( 0, label.find_first_of( ':' ) ) };
                bb.codeBlock.push_back( "jumpi @" + id );
                // basicBlocks.push_back( move( bb ) );
                addBlock( move( bb ) );

                // and start a new one
                bb.codeBlock.reserve( 1 );
                bb.codeBlock.push_back( label );
                bb.name = id;
                bb.startIdx = i;
                bb.endIdx = 0;
            }
            else {
                bb.codeBlock.push_back( buffer->at( i ) );
                auto label{ buffer->at( i ) };
                bb.name = label.substr( 0, label.find_first_of( ':' ) );
                bb.startIdx = i;
                blockStarted = true;
            }
        }
        else if( blockStarted && isJumpInsr( buffer->at( i ) ) ) {
            bb.codeBlock.push_back( buffer->at( i ) );
            bb.endIdx = i;
            // basicBlocks.push_back( move( bb ) );
            addBlock( move( bb ) );
            bb.codeBlock.reserve( 1 );
            blockStarted = false;
            bb.startIdx = 0;
            bb.endIdx = 0;
        }
        else if( blockStarted ) {
            bb.codeBlock.push_back( buffer->at( i ) );
        }
    }
}

bool
Cfg::isJumpInsr( string s ) {

    // SORTED!
    vector< string > jumpInsrs { { 
        "call",
        "cbreq",
        "cbrneq",
        "hlt",
        "jump",
        "jumpi",
        "ret",
        "showb",
        "showi",
        "shows" 
    } };

    if( s[ 0 ] == '#' ) {
        return false;
    }

    auto insr{ s.substr( 0, s.find_first_of( ' ' ) ) };

    if( binary_search( jumpInsrs.begin(), jumpInsrs.end(), insr ) ) {
        return true;
    }

    return false;
}
