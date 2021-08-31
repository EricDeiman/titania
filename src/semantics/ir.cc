#include <algorithm>
#include <cctype>
#include <cstddef>
#include <regex>
#include <string>
#include <unordered_map>

#include "ilocpat.hh"
#include "ir.hh"

using namespace std::literals::string_literals;

// llvnMeta -----------------------------------------------------------------------------

LvnMeta::LvnMeta() {
    renameRegs.resize( 255 );
    for( auto i = 0; i < renameRegs.size(); i++ ) {
        renameRegs[ i ] = i;
    }
}

bool
LvnMeta::lookUp( std::string k, LvnMetaDatum &out ) {
    if( table.count( k ) == 0 ) {
        return false;
    }

    out = table[ k ];
    return true;
}

LvnMetaDatum
LvnMeta::add( std::string k, bool constant, int value ) {
    LvnMetaDatum datum{ number, constant, value  };
    table[ k ] = datum;
    chkCap();
    numToName[ number ].push_back( k );
    number++;
    return datum;
}

void
LvnMeta::clearDest( std::string k ) {
    for( auto &n : numToName ) {
        if( std::find( n.begin(), n.end(), k ) != n.end() ) {
            n.erase( std::remove( n.begin(), n.end(), k ) );
        }
    }
}

size_t
LvnMeta::set( std::string k, size_t v, bool constant, int value ) {
    table[ k ] = { v, constant, value };
    chkCap( v );

    // k is getting assign a new value.  Before adding the new assignment, remove k
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
        auto right{ std::stoi( mg[ 2 ] ) };
        LvnMetaDatum klvn;
        lookUp( key, klvn );
        auto lname{ lookUpName( klvn.valNumber ) };
        auto rname{ lookUpName( right ) };

        return "i2i " + lname + " => " + rname;
    }

    return key;
}

std::string
commafy( std::vector< std::string > ns ) {
    std::string buffer;

    if( ns.size() ) {
        for( auto i = ns.begin(); i != ns.end() - 1; i++ ) {
            buffer += *i + ", ";
        }
        buffer += *( ns.end() - 1 );
    }

    return buffer;
}

std::ostream &
LvnMeta::dumpLvn( std::ostream &os ) {

    os << "local value numbering internals" << std::endl;
    
    os << "\tlvn table" << std::endl;
    for( auto i : table ) {
        os << "\t\t" << i.first << " " << i.second.valNumber << std::endl;
    }

    os << "\tnumber to name table" << std::endl;
    for( auto i = 0; i < numToName.size(); i++ ) {
        os << "\t\t" << i << " " << commafy( numToName[ i ] );
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

void
LvnMeta::renameReg( std::string oldName, std::string newName ) {
    auto oldNum{ stoi( oldName ) };
    auto newNum{ stoi( newName.substr( 1 ) ) };

    renameRegs[ oldNum ] = newNum;
}

std::string
LvnMeta::updateRegisters( std::string i ) {
    auto dirty{ true };
    auto changed{ false };
    auto copy{ i };

    while( dirty ) {
        dirty = false;

        auto regCrx{ std::regex{ regC, std::regex::optimize } };
        auto copyend{ copy.end() };

        if( auto p = copy.find( '#' ) != std::string::npos ) {
            copyend = copy.begin() + p;
        } 

        std::regex_iterator< std::string::iterator > rxIter{ copy.begin(), copyend, regCrx };
        std::regex_iterator< std::string::iterator > rxEnd;

        while( rxIter != rxEnd ) {
            std::string regNum{ ( *rxIter )[ 1 ] };
            if( std::isdigit( regNum[ 0 ] ) ) {
                auto regIdx{ std::stoi( regNum ) };
                if( renameRegs[ regIdx ] != regIdx ) {
                    auto newReg{ "r" +  std::to_string( renameRegs[ regIdx ] ) };
                    auto first{ copy.find( "r" + regNum ) };
                    copy.replace( first, regNum.length() + 1, newReg );
                    dirty = true;
                    changed = true;
                    break;
                }
            }
            rxIter++;
        }
    }

    if( changed ) {
        return copy + "  # " + i;
    }

    return copy;
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

bool
IR::isCommumative( std::string op ) {
    return std::binary_search( commumativeOp.begin(), commumativeOp.end(), op );
}

bool
IR::handleBinOp( LvnMeta &lvn,
                 std::string rator, 
                 std::string lrand, 
                 std::string rrand, 
                 std::string destn,
                 std::string &replacement ) {

    auto hasReplacement{ false };

    LvnMetaDatum llvn;
    if( !lvn.lookUp( lrand, llvn ) ) {
        if( std::isdigit( lrand[ 0 ] ) ) {
            auto v{ std::stoi( lrand ) };
            llvn = lvn.add( lrand, true, v );
        }
        else {
            llvn = lvn.add( lrand );
        }
    }

    LvnMetaDatum rlvn;
    if( !lvn.lookUp( rrand, rlvn ) ) {
        if( std::isdigit( rrand[ 0 ] ) ) {
            auto v{ std::stoi( rrand ) };
            rlvn = lvn.add( rrand, true, v );
        }
        else {
            rlvn = lvn.add( rrand );
        }
    }

    LvnMetaDatum klvn;
    // if llvn and rlvn are both constants, fold them
    if( llvn.constant && rlvn.constant && ( rator == "add" || rator == "sub" || 
            rator == "mult" || rator == "div" ) ) {
        int result;
        if( rator == "add" ) {
            result = llvn.value + rlvn.value;
        }
        else if( rator == "sub" ) {
            result = llvn.value - rlvn.value;
        }
        else if( rator == "mult" ) {
            result = llvn.value * rlvn.value;
        }
        else if( rator == "div" ) {
            result = llvn.value / rlvn.value;
        }
        klvn = lvn.add( std::to_string( result ), true, result );
        replacement = "loadi " + std::to_string( result ) + " => " + "r" + destn;
        hasReplacement = true;
    }
    // if llvn or rlvn is a constant, check for identities
    else if( ( llvn.constant || rlvn.constant ) && ( rator == "add" || rator == "sub" || 
            rator == "mult" || rator == "div" ) ) {
        if( rator == "add" ) {
            // a + 0 = a; 0 + a = a
            if( rlvn.constant && rlvn.value == 0 ) {
                lvn.renameReg( destn, lrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + lrand;
                return true;
            }
            else if( llvn.constant && llvn.value == 0 ) {
                lvn.renameReg( destn, rrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + rrand;
                return true;
            }
        }
        else if( rator == "sub" ) {
            // a - 0 = a
            if( rlvn.constant && rlvn.value == 0 ) {
                lvn.renameReg( destn, lrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + lrand;
                return true;
            }
        }
        else if( rator == "mult" ) {
            // a * 1 = a; 1 * a = a 
            if( rlvn.constant && rlvn.value == 1 ) {
                lvn.renameReg( destn, lrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + lrand;
                return true;
            }
            else if( llvn.constant && llvn.value == 1 ) {
                lvn.renameReg( destn, rrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + rrand;
                return true;
            }
        }
        else if( rator == "div" ) {
            // a / 1 = a
            if( rlvn.constant && rlvn.value == 1 ) {
                lvn.renameReg( destn, lrand.substr( 1 ) );
                replacement = "# renamed register r" + destn + " to " + lrand;
                return true;
            }
        }
    }
    else {
        auto key{ std::to_string( llvn.valNumber ) + rator + 
                std::to_string( rlvn.valNumber ) };
        if( !lvn.lookUp( key, klvn ) ) {
            klvn = lvn.add( key );
        }
        else {
            replacement = lvn.replaceWithI2i( key );
            hasReplacement = true;
        }

        if( isCommumative( rator ) ) {
            auto key2{ std::to_string( rlvn.valNumber ) + rator + 
                    std::to_string( llvn.valNumber ) };
            LvnMetaDatum klvn2;
            if( !lvn.lookUp( key2, klvn2 ) ) {
                klvn2 = lvn.add( key2 );
            }
        }
    }

    lvn.set( "r"s + destn, klvn.valNumber, klvn.constant, klvn.value );

    return hasReplacement;
}

bool
IR::handleLoadiOp( LvnMeta &lvn, std::string imm, std::string reg, std::string &replacement ) {
    LvnMetaDatum ilvn;
    if( !lvn.lookUp( imm, ilvn ) ) {
        ilvn = lvn.add( imm, true, std::stoi( imm ) );
        lvn.set( "r"s + reg, ilvn.valNumber, ilvn.constant, ilvn.value );
        return false;
    }
    else {
        auto newRegName = lvn.lookUpName( ilvn.valNumber );
        lvn.renameReg( reg, newRegName );
        replacement = "# renamed register r" + reg + " to " + newRegName;
        return true;
    }


}

LvnMeta
IR::localValueNumbering( BasicBlock &block ) {
    LvnMeta lvn;

    // find instructions of the form <op> <something>, <something> => <register>       
    auto binOp{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
    auto loadiOp{ mkRX( { R"(loadi +)", immC, arw_, regC } ) };
    std::smatch mg;

    auto line = 0;

    for( auto &i : block.codeBlock ) {
        line++;
        i.assign( lvn.updateRegisters( i ) );
        std::string newLine;
        auto hasRewrite{ false };
        if( std::regex_match( i, mg, loadiOp ) ) {
            hasRewrite = handleLoadiOp( lvn, mg[ 1 ], mg[ 2 ], newLine );
        }
        else if( std::regex_match( i, mg, binOp ) ) { 
            hasRewrite = handleBinOp( lvn, mg[ 1 ], mg[ 2 ], mg[ 3 ], mg[ 4 ], newLine );
        }

        if( hasRewrite ) {
            i.assign( newLine + "  # " + i );
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
                    lvn.dumpLvn( os );
                    os << "adjusted code:" << std::endl;
                    for( auto i : b.codeBlock ) {
                        os << "\t" << i << std::endl;
                    }
                    break;
                }
            }
            break;
        }
    }

    return os;
}