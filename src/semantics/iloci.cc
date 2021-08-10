// An interpreter for my variation of the iloc IR

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

#include "iloci.hh"

// For now, each function gets its own set of registers

std::string immC{ R"((-?\d+|@\w+))" };  // an immediate can be a number or @identifier
std::string regC{ R"(r(\d+|tos|arp))" };
std::string ccC{ R"(cc(\d+))" };

std::string arw_{ " *=> *" };
std::string srw_{ " *-> *" };
std::string cel_{ R"((.*#.*)?)" };  // comment to end of line
std::string com_{ " *, *" };

std::int64_t *
getReg( std::string regSpec, State &state ) {
    if( regSpec == "tos" ) {
        return &state.tos;
    }
    else if( regSpec == "arp" ) {
        return &state.arp;
    }
    else {
        auto reg{ std::stoi( regSpec ) };
        return &state.registers[ reg ];
    }
}

std::int64_t
getImm( std::string immSpec, State &state ) {
    if( immSpec[ 0 ] == '@' ) {
        auto id{ immSpec.substr( 1 ) };
        return state.labelOffsets[ id ];
    }
    else {
        return std::stoi( immSpec );
    }
}

std::regex
mkRX( std::vector< std::string > pattern ) {
    std::string buff;

    for( auto s : pattern ) {
        buff += s;
    }

    buff += cel_;

    return std::regex{ buff, std::regex::optimize };
}

std::string
mkIRPat( std::vector< std::string > irs ) {
    std::string buffer{ '(' };

    for( auto i = 0; i < irs.size(); i++ ) {
        buffer += irs[ i ];
        if( i < irs.size() - 1 ) {
            buffer += "|";
        }
    }

    buffer += ") ";

    return buffer;
}

void doAddOp( std::string line, State &state ) {   // add and subtract
    auto add{ mkRX( { "add ", regC, com_, regC, arw_, regC } ) };
    auto addi{ mkRX( { "addi ", regC, com_, immC, arw_, regC } ) };
    auto sub{ mkRX( { "sub ", regC, com_, regC, arw_, regC} ) };
    auto subi{ mkRX( { "subi ", regC, com_, immC, arw_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, add ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto dest{ getReg( sm[ 3 ], state ) };

        *dest = *reg1 + *reg2;
    }
    else if( std::regex_match( line, sm, addi ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };
        auto imm{ getImm( sm[ 2 ], state ) };
        auto dst{ getReg( sm[ 3 ], state ) };

        *dst = *reg + imm;
    }
    else if( std::regex_match( line, sm, sub ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto dest{ getReg( sm[ 3 ], state ) };

        *dest = *reg1 - *reg2;
    }
    else if( std::regex_match( line, sm, subi ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };
        auto imm{ getImm( sm[ 2 ], state ) };
        auto dst{ getReg( sm[ 3 ], state ) };

        *dst = *reg - imm;
    }
}

void doCall( std::string line, State &state ) {  // call and return
    auto call{ mkRX( { "call ", regC } ) };
    auto rtrn{ mkRX( { "ret" } ) };
    std::smatch sm;

    // how to save and restore registers for the functions?

    if( std::regex_match( line, sm, call ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };

        state.insrPtr = *reg;
    }
    else if( std::regex_match( line, sm, rtrn ) ) {
        // use the arp to find the return address
        state.insrPtr = state.memory[ state.arp - 1 ];
    }
}

void doCbrOp( std::string line, State &state ) {  // conditional branch op
    auto pat{ mkIRPat( { "cbrneq", "cbreq" } ) };
    auto cbr{ mkRX( { pat, ccC, srw_, immC, com_, immC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, cbr ) ) {
        auto cc{ std::stoi( sm[ 2 ] ) };
        auto d1{ getImm( sm[ 3 ], state ) };
        auto d2{ getImm( sm[ 4 ], state ) };

        if( sm[ 1 ] == "cbrneq" ) {
            state.insrPtr = d1;
        }
        else if( sm[ 1 ] == "cbreq" ) {
            state.insrPtr = d2;
        }
    }
}

void doCmp( std::string line, State &state ) {   // comparison op
    auto insrs{ mkIRPat( { "cmpeq", "cmpge", "cmpgt", "cmple", "cmplt", "cmpne", } ) };
    auto ir{ mkRX( { insrs, regC, com_, regC, arw_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, ir ) ) {
        auto reg1{ getReg( sm[ 2 ], state ) };
        auto reg2{ getReg( sm[ 3 ], state ) };
        auto reg3{ getReg( sm[ 4 ], state ) };

        if( sm[ 1 ] == "cmpeq" ) {
            *reg3 = ( *reg1 == *reg2 ) ? 1 : 0;
        }
        else if( sm[ 1 ] == "cmpge" ) {
            *reg3 = ( *reg1 >= *reg2 ) ? 1 : 0;
        }
        else if( sm[ 1 ] == "cmpgt" ) {
            *reg3 = ( *reg1 > *reg2 ) ? 1 : 0;
        }
        else if( sm[ 1 ] == "cmple" ) {
            *reg3 = ( *reg1 <= *reg2 ) ? 1 : 0;
        }
        else if( sm[ 1 ] == "cmplt" ) {
            *reg3 = ( *reg1 < *reg2 ) ? 1 : 0;
        }
        else if( sm[ 1 ] == "cmpne" ) {
            *reg3 = ( *reg1 != *reg2 ) ? 1 : 0;
        }
    }
}

void doComp( std::string line, State &state ) {  // comparison op
    auto comp{ mkRX( { "comp ", regC, com_, regC, arw_, ccC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, comp ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto cc{ std::stoi( sm[ 3 ] ) };

        state.ccregs[ cc ] = ( *reg1 == *reg2 ) ? 1 : 0;
    }
}

void doHlt( std::string line, State &state ) {
    state.running = false;
}

void doI2i( std::string line, State &state ) {
    auto pat{ mkRX( { "i2i ", regC, arw_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, pat ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };

        *reg2 = *reg1;
    }
}

void doJump( std::string line, State &state ) {   // jump ops
    auto jump{ mkRX( { "jump ", regC } ) };
    auto jumpi{ mkRX( { "jumpi ", immC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, jump ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };

        state.insrPtr = *reg;
    }
    else if( std::regex_match( line, sm, jumpi ) ) {
        auto imm{ getImm( sm[ 1 ], state ) };

        state.insrPtr = imm;
    }
}

void doLoad( std::string line, State &state ) {
    // what instruction name was used?
    auto pati{ mkRX( { "loadi ", immC, arw_, regC } ) };
    auto pat{ mkRX( { "load ", regC, arw_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, pati ) ) {
        auto imm{ getImm( sm[ 1 ], state ) };
        auto dst{ getReg( sm[ 2 ], state ) };

        *dst = imm;
    }
    else if( std::regex_match( line, sm, pat ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };

        *reg2 = state.memory[ *reg1 ];
    }
}

void doMultOp( std::string line, State &state ) {    // mult, div, and mod
    auto div{ mkRX( { "div ", regC, com_, regC, arw_, regC } ) };
    auto mod{ mkRX( { "mod ", regC, com_, regC, arw_, regC } ) };
    auto mult{ mkRX( { "mult ", regC, com_, regC, arw_, regC } ) };
    auto multi{ mkRX( { "multi ", regC, com_, immC, arw_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, div ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto dest{ getReg( sm[ 3 ], state ) };

        *dest = *reg1 / *reg2;
    }
    else if( std::regex_match( line, sm, mod ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto dest{ getReg( sm[ 3 ], state ) };

        *dest = *reg1 % *reg2;
    }
    else if( std::regex_match( line, sm, mult ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };
        auto dest{ getReg( sm[ 3 ], state ) };

        *dest = *reg1 * *reg2;
    }
    else if( std::regex_match( line, sm, multi ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };
        auto imm{ getImm( sm[ 2 ], state) };
        auto dst{ getReg( sm[ 3 ], state ) };

        *dst = *reg * imm;
    }
}

void doNot( std::string line, State &state ) {}

void doPop( std::string line, State &state ) {
    auto popr{ mkRX( { "pop ", regC } ) };
    auto pop_{ mkRX( { "pop" } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, popr ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };

        *reg = state.memory[ state.tos ];
    }

    state.tos--;
}

void doPush( std::string line, State &state ) {
    auto pati{ mkRX( { "pushi ", immC } ) };
    auto pat{ mkRX( { "push ", regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, pati ) ) {
        auto imm{ getImm( sm[ 1 ], state ) };

        state.memory[ state.tos++ ] = imm;
    }
    else if( std::regex_match( line, sm, pat ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };

        state.memory[ state.tos++ ] = *reg;
    }
}

void doStore( std::string line, State &state ) {
    auto pat{ mkRX( { "store ", regC, arw_, regC } ) };
    auto patao{ mkRX( { "storeao ", regC, arw_, regC, com_, regC } ) };
    std::smatch sm;

    if( std::regex_match( line, sm, pat ) ) {
        auto src{ getReg( sm[ 1 ], state ) };
        auto dst{ getReg( sm[ 2 ], state ) };

        state.memory[ *dst ] = *src;
    }
    else if( std::regex_match( line, sm, patao ) ) {
        auto src{ getReg( sm[ 1 ], state ) };
        auto base{ getReg( sm[ 2 ], state ) };
        auto off{ getReg( sm[ 3 ], state ) };

        state.memory[ *base + *off ] = *src;
    }
}

void doTos( std::string line, State &state ) {
    auto pat{ mkRX( { "incrtos ", immC } ) }; 
    std::smatch sm;

    if( std::regex_match( line, sm, pat ) ) {
        auto imm{ getImm( sm[ 1 ], state ) };
        assert( imm == 1 || imm % 8 == 0 );

        if( imm == 1 ) {
            state.tos += imm;
        }
        else {
            state.tos += imm / 8;
        }
    }
}


std::unordered_map< std::string, void (*)( std::string, State & ) >
opTable {
    { "add", doAddOp },
    { "addi", doAddOp },
    { "call", doCall },
    { "cbreq", doCbrOp },
    { "cbrneq", doCbrOp },
    { "cmpeq", doCmp },
    { "cmpge", doCmp },
    { "cmpgt", doCmp },
    { "cmple", doCmp },
    { "cmplt", doCmp },
    { "cmpne", doCmp },
    { "comp", doComp },
    { "div", doMultOp },
    { "hlt", doHlt },
    { "i2i", doI2i },
    { "incrtos", doTos },
    { "jumpi", doJump },
    { "load", doLoad },
    { "loadi", doLoad },
    { "mod", doMultOp },
    { "mult", doMultOp },
    { "multi", doMultOp },
    { "not", doNot },
    { "pop", doPop },
    { "push", doPush },
    { "pushi", doPush },
    { "ret", doCall },
    { "store", doStore },
    { "storeao", doStore },
    { "sub", doAddOp },
    { "subi", doAddOp },
};

void
run( std::vector< std::string > program, State &state ) {

    while( state.running ) {
        auto s{ program[ state.insrPtr ] };
        state.insrPtr++;
        if( s[ 0 ] != '#' && s.find_first_of( ':' ) == std::string::npos ) {
            auto key = s.substr( 0, s.find_first_of( ' ' ) );
            ( opTable [ key ] )( s, state );
        }
    }
}

// Read the program from the input stream; break it into lines; trim whitespace from the
// beginning of lines; find the location of all the labels; track the largest number of
// registers used(?).  Leave comments and labels in the program to ease debugging
std::tuple< std::vector< std::string >, State >
prepare( std::istream &in ) {
    State state;
    std::vector< std::string > program;
    char buffer[ 1024 ];

    while( in.getline( buffer, sizeof( buffer ) ) ) {
        std::string tmp{ buffer };

        // trim leading whitespace
        auto ws = 0;
        for( auto c : tmp ) {
            if( c == ' ' || c == '\t' ) {
                ws++;
            }
            else {
                break;
            }
        }

        if( 0 < ws ) {
            tmp.erase( 0, ws );
        }

        program.push_back( tmp );

        if( tmp.find_first_of( ':' ) != std::string::npos ) {
            state.labelOffsets[ tmp.substr( 0, tmp.length() - 1 ) ] = program.size();
        }

    }

    return std::make_tuple( program, state );
}

int main( int argc, char **argv ) {

    // argv[ 1 ] is the iloc text file to interpret
    // read in the entire file
    // scan the IR to find where the labels are and where the @constants are
    if( argc != 2 ) {
        std::cout << "requires an iloc file to run" << std::endl;
        return 64;  // see bsd sysexits.h for more info
    }

    std::ifstream in( argv[ 1 ] );
    if( !in ) {
        std::cout << "problem opening file " << argv[ 1 ] << std::endl;
        return 66;
    }

    auto [ code, state ] = prepare( in );
    run( code, state );

    return 0;
}