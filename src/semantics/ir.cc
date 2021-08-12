#include <algorithm>
#include <cstddef>
#include <regex>
#include <string>
#include <unordered_map>

#include "ilocpat.hh"
#include "ir.hh"

using namespace std::literals::string_literals;

// llvnMeta -----------------------------------------------------------------------------

int
LvnMeta::lookUp( std::string k ) {
    if( table.count( k ) == 0 ) {
        return -1;
    }

    return table[ k ];
}

size_t
LvnMeta::add( std::string k ) {
    table[ k ] = number;
    chkCap();
    numToName[ number ].push_back( k );
    return number++;
}

void
LvnMeta::clearDest( std::string k ) {
    for( auto &n : numToName ) {
        for( auto i = n.begin(); i != n.end(); i++ ) {
            if( *i == k ) {
                *i = "";
            }
        }
    }
}

size_t
LvnMeta::set( std::string k, size_t v ) {
    table[ k ] = v;
    chkCap( v );

    // k is getting assign a new value.  Before adding the new addignment, remove k
    // from any other assignment
    clearDest( k );
    numToName[ v ].push_back( k );

    return v;
}

std::string
LvnMeta::lookUpName( size_t val ) {
    auto vec{ numToName[ val ] };

    for( auto n : vec ) {
        if( n[ 0 ] == 'r' ) {
            return n;
        }
    }

    return ""s;
}

std::string
LvnMeta::replaceWithI2i( std::string key ) {
    auto pat{ R"((\d+)\w+(\d+))" };
    std::regex rx{ pat, std::regex::optimize };
    std::smatch mg;

    if( std::regex_match( key, mg, rx ) ) {
        auto left{ std::stoi( mg[ 1 ] ) };
        auto right{ std::stoi( mg[ 2 ] ) };

        auto lname{ lookUpName( left ) };
        auto rname{ lookUpName( right ) };

        return "i2i " + lname + " => " + rname;
    }

    return key;
}

std::ostream &
LvnMeta::dumpLvn( std::ostream &os ) {

    os << "local value numbering internals" << std::endl;
    
    os << "\tlvn table" << std::endl;
    for( auto i : table ) {
        os << "\t\t" << i.first << " " << i.second << std::endl;
    }

    os << "\tnumber to name table" << std::endl;
    for( auto i = 0; i < numToName.size(); i++ ) {
        os << "\t\t" << i << " ";
        for( auto j : numToName[ i ] ) {
            os << j << ", ";
        }
        os << std::endl;
    }

    return os;
}

void
LvnMeta::chkCap( int c ) {
    int target;

    if( c == -1 ) {
        target = number;
    }
    else {
        target = c;
    }

    if( numToName.size() == target ) {
        numToName.resize( numToName.size() + capFactor );
    }

}


// IR -----------------------------------------------------------------------------------

void
IR::mkBasicBlocks() {
    for( auto &b : fnBuffers ) {
        mkBasicBlock( b );
    }
}

std::ostream&
IR::dumpBasicBlocks( std::ostream &os ) {
    for( auto &b : fnBuffers ) {

        os << b.getName() << std::endl;
        for( auto &bb : b.getBlocks() ) {
            os << "\t" << bb.name << " start: " << bb.startIdx + 1 << ", end: " 
                << bb.endIdx + 1 << std::endl;
            for( auto s : bb.codeBlock ) {

                os << "\t\t" << s << std::endl;
            }
        }
    }

        return os;
}

void
IR::mkBasicBlock( CodeBuffer &code ) {
    auto buffer{ code.getBuffer() };
    BasicBlock bb;
    bool blockStarted = false;

    for( auto i = 0; i < buffer->size(); i++ ) {
        if( buffer->at( i ).find_first_of( ':' ) != std::string::npos ) {
            if( blockStarted ) {
                // finish the existing block
                bb.endIdx = i - 1;
                auto label{ buffer->at( i ) };
                auto id{ label.substr( 0, label.find_first_of( ':' ) ) };
                bb.codeBlock.push_back( "jumpi @" + id );
                code.getBlocks().push_back( std::move( bb ) );

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
            code.getBlocks().push_back( std::move( bb ) );
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
IR::isJumpInsr( std::string s ) {

    // SORTED!
    std::vector< std::string > jumpInsrs { { 
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

    if( std::binary_search( jumpInsrs.begin(), jumpInsrs.end(), insr ) ) {
        return true;
    }

    return false;
}

LvnMeta
IR::localValueNumbering( BasicBlock &block ) {
    LvnMeta lvn;

    // find instructions of the form <op> <something>, <something> => <register>       
    auto inst{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
    std::smatch mg;

    for( auto &i : block.codeBlock ) {
        if( std::regex_match( i, mg, inst ) ) {
            std::string rator{ mg[ 1 ] };
            std::string lrand{ mg[ 2 ] };
            std::string rrand{ mg[ 3 ] };
            std::string destn{ mg[ 4 ] };

            auto llvn{ lvn.lookUp( lrand ) };
            if( llvn == -1 ) {
                llvn = lvn.add( lrand );
            }

            auto rlvn{ lvn.lookUp( rrand ) };
            if( rlvn == -1 ) {
                rlvn = lvn.add( rrand );
            }

            auto key{ std::to_string( llvn ) + rator + std::to_string( rlvn ) };
            auto klvn{ lvn.lookUp( key ) };
            if( klvn == -1 ) {
                klvn = lvn.add( key );
            }
            else {
                std::cout << "can replace line " << i << " with " << lvn.replaceWithI2i( key ) << std::endl;
                // i.assign( lvn.replaceIns( key ) );
            }

            lvn.set( "r"s + destn, klvn );
        }
    }

    return lvn;
}

std::ostream &
IR::testLocalValueNumbering( std::string fnName, 
                             std::string blockName, 
                             std::ostream &os ) {


    for( auto &x : fnBuffers ) {
        if( x.getName() == fnName ) {
            for( auto &b : x.basicBlocks ) {
                if( b.name == blockName ) {
                    auto lvn{ localValueNumbering( b ) };
                    // lvn.dumpLvn( os );
                    break;
                }
            }
            break;
        }
    }



    return os;
}