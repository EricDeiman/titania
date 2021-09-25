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
#include <cstddef>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

using namespace std;

#include "cfg.hh"
#include "ilocpat.hh"

Cfg::Cfg( CodeBuffer &fnCode, string fName ) {
    fileName = fName;
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

            checkDataFlow( buffer->at( i ), bb );

            bb.endIdx = i;
            auto oldBbName = bb.name;
            addBlock( move( bb ) );
            bb.codeBlock.reserve( 1 );
            blockStarted = false;
            bb.startIdx = 0;
            bb.endIdx = 0;
            auto targets{ getJumpTargets( buffer->at( i ) ) };
            for( auto t : targets ) {
                Edge out{ .source = oldBbName, .destination = t };
                edges.push_back( move( out ) );
            }
        }
        else if( blockStarted ) {
            bb.codeBlock.push_back( buffer->at( i ) );

            checkDataFlow( buffer->at( i ), bb );
        }
    }

    // Calculate the fan-in for each basic block
    for( auto e : edges ) {
        auto offset{ blockNames[ e.destination ] };
        basicBlocks[ offset ].fanIn++;
    }

    drawBbs();

    findExtendedBasicBlocks();

    drawEbbs();
}

string
htmlEncode( string html ) {
    while( html.find( '>' ) != string::npos ) {
        auto pos{ html.find( '>' ) };
        html.replace( pos, 1, "&gt;" );
    }

    return html;
}

void
Cfg::drawBbs() {
    auto fName{ fileName.substr( 0, fileName.find( '.' ) ) };
    ofstream out{ fName + ".cfg.dot" };

    out << "digraph " << fName << " {" << endl;
    out << "\tnode[ shape = box ];" << endl;

    unordered_map< int, vector< string > > ranks;
    auto thenPat{ R"(thenBody(\d+))" };
    auto elsePat{ R"(elseBody(\d+))" };
    auto thenRx{ regex{ thenPat, regex::optimize } };
    auto elseRx{ regex{ elsePat, regex::optimize } };
    smatch mg;

    for( auto b : basicBlocks ) {
        out << "\t\"" << b.name << R"(" [label = <)" << endl;
        out << R"(<table border="0" cellborder="0">)" << endl;

        for( auto i : b.codeBlock ) {
            out << R"(<tr><td align="left">)" << htmlEncode( i ) << "</td></tr>" << endl;
        }

        out << "</table>" << endl;
        out << ">];" << endl;

        if( regex_match( b.name, mg, thenRx ) ) {
            ranks[ stoi( mg[ 1 ] ) ].push_back( b.name );
        }

        if( regex_match( b.name, mg, elseRx ) ) {
            ranks[ stoi( mg[ 1 ] ) ].push_back( b.name );
        }
    }
    out << endl;

    for( auto p : ranks ) {
        out << "\t{ rank = same; ";
        for( auto n : p.second ) {
            out << "\"" << n << "\"; ";
        }
        out << "};" << endl;
    }
    out << endl;

    for( auto e : edges ) {
        out << "\t\"" << e.source << "\" -> \"" << e.destination << "\";" << endl;
    }

    out << "}" << endl;
}

string
formatEbb( vector< string > ebb ) {
    auto buffer{ "("s };

    for( auto i = 0; i < ebb.size() - 1; i++ ) {
        buffer += ebb[ i ] + ", "s;
    }

    buffer += ebb[ ebb.size() - 1 ];

    buffer += ")"s;

    return buffer;
}

void
Cfg::drawEbbs() {
    auto fName{ fileName.substr( 0, fileName.find( '.' ) ) };
    ofstream out{ fName + ".ebbs.dot" };

    out << "digraph " << fName << " {" << endl;
    out << "\tnode[ shape = box ];" << endl;

    for( auto ebb : extendedBasicBlocks ) {
        out << "\t\"" << formatEbb( ebb ) << "\";" << endl;
    }

    out << "}" << endl;    
}

void
Cfg::findExtendedBasicBlocks() {
    // A leader is a basic block with a fan-in > 1
    vector< string >leaders;

    for( auto b : basicBlocks ) {
        if( 1 < b.fanIn ) {
            leaders.push_back( b.name );
        }
    }

    for( auto n : leaders ) {
        vector< string >ebb;
        ebb.push_back( n );
    
        buildEbb( ebb, n );    
    
        extendedBasicBlocks.push_back( move( ebb ) );
    }  
}

vector< string > &
Cfg::buildEbb( vector< string > &ebb, string blockName ) {

    for( auto e : edges ) {
        if( e.source == blockName ) {
            auto offset{ blockNames[ e.destination ] };
            if( basicBlocks[ offset ].fanIn == 1 ) {
                ebb.push_back( e.destination );
                buildEbb( ebb, e.destination );
            }
        }
    }

    return ebb;
}

vector< string >
Cfg::getJumpTargets( string instruction ) {
    vector< string > result;

    auto opcode{ instruction.substr( 0, instruction.find( ' ' ) ) };

    if( opcode != "hlt" ) {
        if( opcode[ 0 ] == 'c' ) {  // conditional branch
            auto pat{ mkRX( { insrC, ccC, srw_, immC, com_, immC } ) };
            smatch mg;
            if( regex_match( instruction, mg, pat ) ) {
                result.push_back( mg[ 3 ].str().substr( 1 ) );
                result.push_back( mg[ 4 ].str().substr( 1 ) );
            }
        }
        else {   // jump instruction
            auto pat{ mkRX( { insrC, immC } ) };
            smatch mg;

            if( regex_match( instruction, mg, pat ) ) {
                result.push_back( mg[ 2 ].str().substr( 1 ) );
            }
        }
    }

    return result;
}

void
Cfg::checkDataFlow( string insr, BasicBlock &bb ) {
    // Process names used by the instruction
    auto touchedNames{ getNamesUsedOrDefined( insr ) };

    if( touchedNames.used.size() > 0 ) {
        bb.namesUsed.insert( touchedNames.used.begin(), touchedNames.used.end() );

        for( auto n : touchedNames.used ) {
            if( 0 == bb.namesDefined.count( n ) ) {
                bb.upExposedNames.insert( n );
            }
        }
    }

    if( touchedNames.defined.size() > 0 ) {
        bb.namesDefined.insert( touchedNames.defined.begin(), 
            touchedNames.defined.end() );
    }
}

Cfg::Names
Cfg::getNamesUsedOrDefined( string insr ) {
    // Names in this context are register names, not labels
    Names rtn;

    // What is the shape of the instruction
    
    // <insr> <name>, <name> => <name>
    auto binOp{ mkRX( { insrC, immOrRegOrCcC, com_, immOrRegOrCcC, arw_, immOrRegOrCcC } ) };
    
    // <insr> <name> => <name>
    auto uniOp{ mkRX( { insrC, immOrRegOrCcC, arw_, immOrRegOrCcC } ) };
    
    // <insr> <name>
    auto simOp{ mkRX( { insrC, immOrRegOrCcC }) };
    
    // <insr> <name> -> <name>, <name>
    auto cbrOp{ mkRX( { insrC, immOrRegOrCcC, srw_, immOrRegOrCcC, com_, immOrRegOrCcC }) };

    smatch mg;

    // Names start with "r" for register or "c" for condition code
    if( regex_match( insr, mg, binOp ) ) {
        if( mg[ 2 ].str()[ 0 ] == 'r' ||  mg[ 2 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 2 ].str() );
        }
        if( mg[ 3 ].str()[ 0 ] == 'r' ||  mg[ 3 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 3 ].str() );
        }
        if( mg[ 4 ].str()[ 0 ] == 'r' ||  mg[ 4 ].str()[ 0 ] == 'c' ) {
            rtn.defined.insert( mg[ 4 ].str() );
        }
    }
    else if( regex_match( insr, mg, uniOp ) ) {
        if( mg[ 2 ].str()[ 0 ] == 'r' ||  mg[ 2 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 2 ].str() );
        }
        // store does not define its RHS
        if( mg[ 1 ].str().find( "store" ) == string::npos ) {
            if( mg[ 3 ].str()[ 0 ] == 'r' ||  mg[ 3 ].str()[ 0 ] == 'c' ) {
                rtn.defined.insert( mg[ 3 ].str() );
            }

        }
    }
    else if( regex_match( insr, mg, simOp ) ) {
        if( mg[ 2 ].str()[ 0 ] == 'r' ||  mg[ 2 ].str()[ 0 ] == 'c' ) {
            // push and jump instructions both use a name
            if( mg[ 1 ].str().find( "push" ) == 0 ||
                mg[ 1 ].str().find( "jump" ) == 0 ) {
                rtn.used.insert( mg[ 2 ].str() );
            }
            // pop defines a name
            else if( mg[ 1 ].str().find( "pop" ) == 0 ) {
                rtn.defined.insert( mg[ 2 ].str() );
            }
        }
    }
    else if( regex_match( insr, mg, cbrOp ) ) {
       if( mg[ 2 ].str()[ 0 ] == 'r' ||  mg[ 2 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 2 ].str() );
        }
        if( mg[ 3 ].str()[ 0 ] == 'r' ||  mg[ 3 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 3 ].str() );
        }
        if( mg[ 4 ].str()[ 0 ] == 'r' ||  mg[ 4 ].str()[ 0 ] == 'c' ) {
            rtn.used.insert( mg[ 4 ].str() );
        }
    }

    return rtn;
}

bool
Cfg::isJumpInsr( string s ) {

    // SORTED!
    vector< string > jumpInsrs { { 
        // "call",  // a call is always followed by a label
        "cbreq",
        "cbrneq",
        "hlt",  // halt always means end of a block
        "jump",
        "jumpi",
        "ret",  // return always means end of a block
        // "showb",
        // "showi",
        // "shows" 
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
