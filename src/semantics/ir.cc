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
#include <cctype>
#include <cstddef>
#include <ostream>
#include <queue>
#include <regex>
#include <string>
#include <unordered_map>

#include "ilocpat.hh"
#include "ir.hh"


using namespace literals::string_literals;

// llvnMeta -----------------------------------------------------------------------------

LvnMeta::LvnMeta() {
    renameRegs.resize( 255 );
    for( auto i = 0; i < renameRegs.size(); i++ ) {
        renameRegs[ i ] = i;
    }
}

bool
LvnMeta::lookUp( string k, LvnMetaDatum &out ) {
    if( table.count( k ) == 0 ) {
        return false;
    }

    out = table[ k ];
    return true;
}

LvnMetaDatum
LvnMeta::add( string k, bool constant, int value ) {
    LvnMetaDatum datum{ number, constant, value  };
    table[ k ] = datum;
    chkCap();
    numToName[ number ].push_back( k );
    number++;
    return datum;
}

void
LvnMeta::clearDest( string k ) {
    for( auto &n : numToName ) {
        if( find( n.begin(), n.end(), k ) != n.end() ) {
            n.erase( remove( n.begin(), n.end(), k ) );
        }
    }
}

size_t
LvnMeta::set( string k, size_t v, bool constant, int value ) {
    table[ k ] = { v, constant, value };
    chkCap( v );

    // k is getting assign a new value.  Before adding the new assignment, remove k
    // from any other assignment
    clearDest( k );
    numToName[ v ].push_back( k );

    return v;
}

string
LvnMeta::lookUpName( size_t val ) {
    auto vec{ numToName[ val ] };

    for( auto n : vec ) {
        if( n[ 0 ] == 'r' ) {
            return n;
        }
    }

    return ""s;
}

string
LvnMeta::replaceWithI2i( string key ) {
    auto pat{ R"((\d+)\w+(\d+))" };
    regex rx{ pat, regex::optimize };
    smatch mg;

    if( regex_match( key, mg, rx ) ) {
        auto right{ stoi( mg[ 2 ] ) };
        LvnMetaDatum klvn;
        lookUp( key, klvn );
        auto lname{ lookUpName( klvn.valNumber ) };
        auto rname{ lookUpName( right ) };

        return "i2i " + lname + " => " + rname;
    }

    return key;
}

string
commafy( vector< string > ns ) {
    string buffer;

    if( ns.size() ) {
        for( auto i = ns.begin(); i != ns.end() - 1; i++ ) {
            buffer += *i + ", ";
        }
        buffer += *( ns.end() - 1 );
    }

    return buffer;
}

ostream &
LvnMeta::dumpLvn( ostream &os ) {

    os << "local value numbering internals" << endl;
    
    os << "\tlvn table" << endl;
    for( auto i : table ) {
        os << "\t\t" << i.first << " " << i.second.valNumber << endl;
    }

    os << "\tnumber to name table" << endl;
    for( auto i = 0; i < numToName.size(); i++ ) {
        os << "\t\t" << i << " " << commafy( numToName[ i ] );
        os << endl;
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

bool
LvnMeta::renameReg( string oldName, string newName ) {
    auto _oldName{ oldName };
    auto _newName{ newName };

    if( _oldName[ 0 ] == 'r' ) {
        _oldName = _oldName.substr( 1 );
    }

    if( _newName[ 0 ] == 'r' ) {
        _newName = _newName.substr( 1 );
    }

    if( isdigit( _oldName[ 0 ] ) && isdigit( _newName[ 0 ] ) ) {
        auto oldNum{ stoi( _oldName ) };
        auto newNum{ stoi( _newName ) };

        renameRegs[ oldNum ] = newNum;
        return true;
    }

    return false;
}

string
LvnMeta::updateRegisters( string i ) {
    auto dirty{ true };
    auto changed{ false };
    auto copy{ i };

    while( dirty ) {
        dirty = false;

        auto regCrx{ regex{ regC, regex::optimize } };
        auto copyend{ copy.end() };

        if( auto p = copy.find( '#' ) != string::npos ) {
            copyend = copy.begin() + p;
        } 

        regex_iterator< string::iterator > rxIter{ copy.begin(), copyend, regCrx };
        regex_iterator< string::iterator > rxEnd;

        while( rxIter != rxEnd ) {
            string regNum{ ( *rxIter )[ 1 ] };
            if( isdigit( regNum[ 0 ] ) ) {
                auto regIdx{ stoi( regNum ) };
                if( renameRegs[ regIdx ] != regIdx ) {
                    auto newReg{ "r" +  to_string( renameRegs[ regIdx ] ) };
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

ostream&
IR::dumpBasicBlocks( ostream &os ) {
    for( auto &b : fnBuffers ) {

        os << b.getName() << endl;
        for( auto &bb : b.getBlocks() ) {
            os << "\t" << bb.name << " start: " << bb.startIdx + 1 << ", end: " 
                << bb.endIdx + 1 << endl;
            for( auto s : bb.codeBlock ) {

                os << "\t\t" << s << endl;
            }
        }
    }

        return os;
}

ostream&
IR::dumpCfgBasicBlocks( ostream &os ) {
    for( auto c : cfgs ) {
        for( auto b : c.basicBlocks ) {
            for( auto i : b.codeBlock ) {
                os << i << endl;
            }
        }
    }
    
    return os;
}

bool
IR::isCommumative( string op ) {
    return binary_search( commumativeOp.begin(), commumativeOp.end(), op );
}

bool
startsWith( string target, string prefix ) {
    if( size_t p = target.find( prefix ) != string::npos ) {
        return p == 1;
    }

    return false;
}

int
IR::operatorPrecedence( string op ) {
    if( startsWith( op, "add" ) || startsWith( op, "mult" ) ) {
        if( startsWith( op, "mult" ) ) {
            return 2;
        }
        else {
            return 1;
        }
    }

    return 0;
}

bool
IR::handleBinOp( LvnMeta &lvn,
                 string rator, 
                 string lrand,  // has prefix r
                 string rrand,  // has preifx r
                 string destn,  // does not have prefix r
                 string &replacement ) {

    auto hasReplacement{ false };

    LvnMetaDatum llvn;
    if( !lvn.lookUp( lrand, llvn ) ) {
        if( isdigit( lrand[ 0 ] ) ) {
            auto v{ stoi( lrand ) };
            llvn = lvn.add( lrand, true, v );
        }
        else {
            llvn = lvn.add( lrand );
        }
    }

    LvnMetaDatum rlvn;
    if( !lvn.lookUp( rrand, rlvn ) ) {
        if( isdigit( rrand[ 0 ] ) ) {
            auto v{ stoi( rrand ) };
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
        klvn = lvn.add( to_string( result ), true, result );
        replacement = "loadi " + to_string( result ) + " => " + "r" + destn;
        hasReplacement = true;
    }
    // if llvn or rlvn is a constant, check for identities
    else if( ( llvn.constant || rlvn.constant ) && ( rator == "add" || rator == "sub" || 
            rator == "mult" || rator == "div" ) ) {
        if( rator == "add" ) {
            // a + 0 = a; 0 + a = a
            if( rlvn.constant && rlvn.value == 0 ) {
                if( lvn.renameReg( destn, lrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + lrand;
                    return true;
                }
            }
            else if( llvn.constant && llvn.value == 0 ) {
                if( lvn.renameReg( destn, rrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + rrand;
                    return true;
                }
            }
        }
        else if( rator == "sub" ) {
            // a - 0 = a
            if( rlvn.constant && rlvn.value == 0 ) {
                if( lvn.renameReg( destn, lrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + lrand;
                    return true;
                }
            }
        }
        else if( rator == "mult" ) {
            // a * 1 = a; 1 * a = a 
            if( rlvn.constant && rlvn.value == 1 ) {
                if( lvn.renameReg( destn, lrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + lrand;
                    return true;
                }
            }
            else if( llvn.constant && llvn.value == 1 ) {
                if( lvn.renameReg( destn, rrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + rrand;
                    return true;
                }
            }
        }
        else if( rator == "div" ) {
            // a / 1 = a
            if( rlvn.constant && rlvn.value == 1 ) {
                if( lvn.renameReg( destn, lrand ) ) {
                    replacement = "# renamed register r" + destn + " to " + lrand;
                    return true;
                }
            }
        }
        // if we get here, there was no transformation
        auto key{ to_string( llvn.valNumber ) + rator + 
                to_string( rlvn.valNumber ) };
        if( !lvn.lookUp( key, klvn ) ) {
            klvn = lvn.add( key );
        }

        if( isCommumative( rator ) ) {
            auto key2{ to_string( rlvn.valNumber ) + rator + 
                    to_string( llvn.valNumber ) };
            LvnMetaDatum klvn2;
            if( !lvn.lookUp( key2, klvn2 ) ) {
                klvn2 = lvn.add( key2 );
            }
        }
    }
    else {
        auto key{ to_string( llvn.valNumber ) + rator + 
                to_string( rlvn.valNumber ) };
        if( !lvn.lookUp( key, klvn ) ) {
            klvn = lvn.add( key );
        }
        else {
            replacement = lvn.replaceWithI2i( key );
            hasReplacement = true;
        }

        if( isCommumative( rator ) ) {
            auto key2{ to_string( rlvn.valNumber ) + rator + 
                    to_string( llvn.valNumber ) };
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
IR::handleLoadiOp( LvnMeta &lvn, string imm, string reg, string &replacement ) {
    LvnMetaDatum ilvn;
    if( !lvn.lookUp( imm, ilvn ) ) {
        ilvn = lvn.add( imm, true, stoi( imm ) );
        lvn.set( "r"s + reg, ilvn.valNumber, ilvn.constant, ilvn.value );
        return false;
    }
    else {
        auto newRegName = lvn.lookUpName( ilvn.valNumber );
        if( lvn.renameReg( reg, newRegName ) ) {
            replacement = "# renamed register r" + reg + " to " + newRegName;
            return true;
        }
        else {
            ilvn = lvn.add( imm, true, stoi( imm ) );
            lvn.set( "r"s + reg, ilvn.valNumber, ilvn.constant, ilvn.value );
            return false;
        }
    }


}

void
IR::doLocalValueNumbering() {
    for( auto &c : cfgs ) {
        for( auto &b : c.basicBlocks ) {
            localValueNumbering( b );
        }
    }
}

LvnMeta
IR::localValueNumbering( BasicBlock &block ) {
    LvnMeta lvn;

    // find instructions of the form <op> <something>, <something> => <register>       
    auto binOp{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
    auto loadiOp{ mkRX( { R"(loadi +)", immC, arw_, regC } ) };
    smatch mg;

    auto line = 0;

    for( auto &i : block.codeBlock ) {
        line++;
        i.assign( lvn.updateRegisters( i ) );
        string newLine;
        auto hasRewrite{ false };
        if( regex_match( i, mg, loadiOp ) ) {
            hasRewrite = handleLoadiOp( lvn, mg[ 1 ], mg[ 2 ], newLine );
        }
        else if( regex_match( i, mg, binOp ) ) { 
            hasRewrite = handleBinOp( lvn, mg[ 1 ], mg[ 2 ], mg[ 3 ], mg[ 4 ], newLine );
        }

        if( hasRewrite ) {
            i.assign( newLine + "  # " + i );
        }
    }

    return lvn;
}

LvnMeta
IR::localValueNumberingWithData( BasicBlock &block, LvnMeta lvn ) {

    // find instructions of the form <op> <something>, <something> => <register>       
    auto binOp{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
    auto loadiOp{ mkRX( { R"(loadi +)", immC, arw_, regC } ) };
    smatch mg;

    auto line = 0;

    for( auto &i : block.codeBlock ) {
        line++;
        i.assign( lvn.updateRegisters( i ) );
        string newLine;
        auto hasRewrite{ false };
        if( regex_match( i, mg, loadiOp ) ) {
            hasRewrite = handleLoadiOp( lvn, mg[ 1 ], mg[ 2 ], newLine );
        }
        else if( regex_match( i, mg, binOp ) ) { 
            hasRewrite = handleBinOp( lvn, mg[ 1 ], mg[ 2 ], mg[ 3 ], mg[ 4 ], newLine );
        }

        if( hasRewrite ) {
            i.assign( newLine + "  # " + i );
        }
    }

    return lvn;
}

void
IR::doSuperLocalValueNumbering() {
    for( auto &c : cfgs ) {
        for( auto &e : c.extendedBasicBlocks ) {
            superLocalValueNumbering( c, e );
        }
    }
}

void
IR::superLocalValueNumbering( Cfg &cfg, vector< string > &extendedBB ) {
    LvnMeta lvn;
    recursiveSLVN( cfg, extendedBB[ 0 ], lvn );
}

void
IR::recursiveSLVN( Cfg &cfg, string bbName, LvnMeta lvnData ) {
    auto offset{ cfg.getOffset( bbName ) };
    auto bb{ cfg.basicBlocks[ offset ] };

    lvnData = localValueNumberingWithData( bb, lvnData );

    // how does the basic block end?  recurse wiht each destination
    auto bbEnd{ *( bb.codeBlock.end() - 1 )  };
        // "cbreq",
        // "cbrneq",
    auto cbrIns{ mkRX( { insrC, ccC, arw_, immC, com_, immC } ) };
        // "jump",
        // "jumpi",
    auto jmpIns{ mkRX( { insrC, immC } ) };
    smatch mg;

    if( regex_match( bbEnd, mg, cbrIns ) ) {
        recursiveSLVN( cfg, mg[ 3 ].str().substr( 1 ), lvnData );
        recursiveSLVN( cfg, mg[ 4 ].str().substr( 1 ), lvnData );
    }
    else if( regex_match( bbEnd, mg, jmpIns ) ) {
        recursiveSLVN( cfg, mg[ 2 ].str().substr( 1 ), lvnData );
    }
}

// bool
// operator>( const pair< string, int > &lhs, const pair< string, int > &rhs ){
//     return lhs.second > rhs.second;
// }

// int
// IR::flatten( string reg,  
//         priority_queue< thbDatum,
//             vector< thbDatum >,
//             greater< thbDatum > > &q ) {


//     return 0;
// }

// void
// IR::rebuild( thbQueue &q, string op ) {

// }

// void
// IR::balance( thbDatum &root ) {

//     if( rank[ root.temporary ] >= 0 ) {
//         return;
//     }

//     auto binOp{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
//     smatch mg;
//     thbQueue q;

//     regex_match( root.instr, mg, binOp );

//     rank[ root.temporary ] = flatten( mg[ 2 ], q ) + flatten( mg[ 3 ], q );

//     rebuild( q, mg[ 1 ] );
// }

// void
// IR::treeHeightBalance( BasicBlock &block ) {

//     auto uses{ buildUses( block ) };
//     auto binOp{ mkRX( { insrC, immOrRegC, com_, immOrRegC, arw_, regC } ) };
//     smatch mg;
//     auto line{ 0 };
//     vector< string >isRoot;
//     thbQueue roots;


//     for( auto &inst : block.codeBlock ) {
//         if( regex_match( inst, mg, binOp ) ) {
//             auto dest{ "r" + mg[ 4 ].str() };
//             rank[ dest ] = -1;
//             if( isCommumative( mg[ 1 ] ) ) {
//                 if( 1 < uses[ dest ].size() || 1 == uses[ dest ].size() && uses[ dest ][ 0 ] != line ) {
//                     // dest is a root
//                     isRoot.push_back( dest );
//                     roots.push( { dest, inst, operatorPrecedence( mg[ 1 ].str() ) } );
//                 }
//             }
//         }        
//         line++;
//     }

//     cout << "in tree height balance" << endl;
//     while( !roots.empty() ) {
//         auto &i = roots.top();

//         roots.pop();

//     }
// }

// ostream &
// IR::testTreeHeightBalance( string fnName, string blockName, ostream &os ) {
//     for( auto &x : fnBuffers ) {
//        if( x.getName() == fnName ) {
//             for( auto &b : x.basicBlocks ) {
//                 if( b.name == blockName ) {
//                     treeHeightBalance( b );
//                     break;
//                 }
//             }
//             break;
//         }
//     }

//     return os;

// }