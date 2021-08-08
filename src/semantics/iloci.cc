// An interpreter for my variation of the iloc IR
#include <fstream>
#include <regex>
#include <unordered_map>

#include "iloci.hh"

// For now, each function gets its own set of registers

std::string regPat{ R"(r([\d]+|tos|arp))" };
std::string celPat{ R"((.*#.*)?)" };  // comment to end of line

size_t *
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

void doAddOp( std::string line, State &state ) {}  // add and subtract

void doCall( std::string line, State &state ) {}  // call and return

void doCbrOp( std::string line, State &state ) {}  // conditional branch op

void doComp( std::string line, State &state ) {}  // compparison op

void doHlt( std::string line, State &state ) {
    state.running = false;
}

void doI2i( std::string line, State &state ) {
    std::regex pat{ "i2i " + regPat + " => " + regPat + celPat,  std::regex::optimize };
    std::smatch sm;

    if( std::regex_match( line, sm, pat ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };

        *reg2 = *reg1;
    }
}

void doJump( std::string line, State &state ) {}  // jump ops

void doLoad( std::string line, State &state ) {
    // what instruction name was used?
    std::regex pati{ R"(loadi ([\d]+) => )" + regPat + celPat,  std::regex::optimize };
    std::regex pat{ "load " + regPat + " => " + regPat + celPat,  std::regex::optimize };
    std::smatch sm;

    if( std::regex_match( line, sm, pati ) ) {
        auto imm{ std::stoi( sm[ 1 ] ) };

        auto dest{ getReg( sm[ 2 ], state ) };
        *dest = imm;
    }
    else if( std::regex_match( line, sm, pat ) ) {
        auto reg1{ getReg( sm[ 1 ], state ) };
        auto reg2{ getReg( sm[ 2 ], state ) };

        *reg2 = state.memory[ *reg1 ];
    }
}

void doMultOp( std::string line, State &state ) {}  // mult, div, and mod

void doNot( std::string line, State &state ) {}

void doPop( std::string line, State &state ) {}

void doPush( std::string line, State &state ) {
    std::regex pati{ R"(pushi ([\d]+))" + celPat,  std::regex::optimize };
    std::regex pat{ "push " + regPat + celPat,  std::regex::optimize };
    std::smatch sm;

    if( std::regex_match( line, sm, pati ) ) {
        auto imm{ std::stoi( sm[ 1 ] ) };
        state.memory[ state.tos++ ] = imm;
    }
    else if( std::regex_match( line, sm, pat ) ) {
        auto reg{ getReg( sm[ 1 ], state ) };
        state.memory[ state.tos++ ] = *reg;
    }
}

void doStore( std::string line, State &state ) {}

void doTos( std::string line, State &state ) {
    std::regex pat{ R"(intrtos ([\d]+))" + celPat,  std::regex::optimize }; 
    std::smatch sm;

    if( std::regex_match( line, sm, pat ) ) {
        auto imm{ std::stoi( sm[ 1 ] ) };
        state.tos += imm / 8;
    }
}


std::unordered_map< std::string, void (*)( std::string, State & ) >
opTable {
    { "add", doAddOp },
    { "addi", doAddOp },
    { "call", doCall },
    { "cbr_eq", doCbrOp },
    { "cbr_neq", doCbrOp },
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
            state.labelOffsets[ tmp.substr( 0, tmp.length() - 1 ) ] = program.size() + 1;
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